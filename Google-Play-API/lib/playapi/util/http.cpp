#include <playapi/util/http.h>

#include <cassert>
#include <zlib.h>
#include <Net/Http/V1/RequestImpl.h>
#include <Net/Http/Request.h>
#include <Net/Http/Response.h>
#include <Net/Http/V1/ResponseImpl.h>
#include <Net/../utility.h>
#include <Net/TLSSocket.h>
#include <cstring>
#include <iostream>

using namespace playapi;

void url_encoded_entity::add_pair(const std::string& key, const std::string& val) {
    pairs.push_back({key, val});
}


std::string url_encoded_entity::encode() const {
    std::stringstream out;
    int i = 0;
    for (auto&& p : pairs) {
        if(i++) {
            out << "&";
        }
        out << Utility::UrlEncode(p.first) << "=" << Utility::UrlEncode(p.second);
    }
    return out.str();
}

http_response::http_response(bool ok, long statusCode, std::string body) :
        ok(ok), statusCode(statusCode), body(std::move(body)) {
    //
}

http_response::http_response(http_response&& r) : ok(r.ok), statusCode(r.statusCode),
                                                  body(r.body) {
    r.ok = false;
    r.statusCode = 0;
    r.body = std::string();
}

http_response& http_response::operator=(http_response&& r) {
    ok = r.ok;
    body = r.body;
    r.ok = false;
    r.body = std::string();
    return *this;
}

http_response::~http_response() {
}

void http_request::set_body(const url_encoded_entity& ent) {
    set_body(ent.encode());
}

void http_request::add_header(const std::string& key, const std::string& value) {
    headers[key] = value;
}

void http_request::set_gzip_body(const std::string& str) {
    z_stream zs;
    zs.zalloc = Z_NULL;
    zs.zfree = Z_NULL;
    zs.opaque = Z_NULL;
    int ret = deflateInit2(&zs, Z_BEST_COMPRESSION, Z_DEFLATED, 31, 8, Z_DEFAULT_STRATEGY);
    assert(ret == Z_OK);

    zs.avail_in = (uInt) str.length();
    zs.next_in = (unsigned char*) str.data();
    std::string out;
    while(true) {
        out.resize(out.size() + 4096);
        zs.avail_out = 4096;
        zs.next_out = (unsigned char*) out.data();
        ret = deflate(&zs, Z_FINISH);
        assert(ret != Z_STREAM_ERROR);
        if (zs.avail_out != 0) {
            out.resize(out.size() - zs.avail_out);
            break;
        }
    }
    deflateEnd(&zs);
    body = std::move(out);
}

http_response http_request::perform() {
    std::stringstream output;
    long status;
    Net::Http::Request req(std::make_shared<Net::Http::V1::RequestImpl>());
    for (auto&& h : headers)
    {
        req.Add(h);
    }
    auto protsep = url.find("://");
    auto pathsep = url.find("/", protsep + 3);
    req.authority = url.substr(protsep + 3, pathsep - (protsep + 3));
    req.path = url.substr(pathsep);
    switch (method)
    {
    case playapi::http_method::GET:
        req.method = "GET";
        break;
    case playapi::http_method::POST:
        req.method = "POST";
        break;
    case playapi::http_method::PUT:
        req.method = "PUT";
        break;
    default:
        break;
    }
    if(user_agent.length())
        req.Add({"User-Agent", user_agent});
    if(encoding.length())
        req.Add({"Content-Encoding", encoding});
    // req.contentlength = body.length();
    req.headerlist.insert({"Content-Length",std::to_string(body.length())});
    req.headerlist.insert({"Host", req.authority});
    std::vector<uint8_t> buffer(102400);
    auto it = buffer.begin();
    req.Encode(it);
    auto payload = req.method + " " + req.path + " HTTP/1.1\r\n" + std::string(buffer.begin(), it) + body.c_str();
    // printf("http request: %s\n%s\n", url.c_str(), payload.c_str());
    
    struct addrinfo hints, *result, *ptr;
	memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    auto sslctx = SSL_CTX_new(TLS_client_method());
    auto sock = Net::TLSSocket::Connect(req.authority, 443, false);
    auto ok = sock->GetOutputStream().SendAll((const uint8_t*)payload.data(), payload.length());
    auto in = sock->GetInputStream();
	if (ok) {
        int i = 0;
        while(true) {
            auto len = in.Receive(buffer.data(), buffer.size());
            auto beg = buffer.cbegin();
            auto end = buffer.cbegin() + len;
            // std::cout << "Received len=" << len << "\n";
            if (len <= 0) {
                break;
            }
            if (!(i++)) {
                // printf("decode resp\n");
                Net::Http::Response resp(std::make_shared<Net::Http::V1::ResponseImpl>());
                resp.Decode(beg, end);
                status = resp.status;
                // printf("Status %ld\n", status);
                //std::search(beg, buffer.begin() + len, )
            }
            auto crlf = "\r\n";
            // printf("search ln\n");

            auto fres = std::search(beg, end, crlf, crlf + 2);
            // printf("found ln ln\n");

            if(fres != end) {
                // printf("std::stoll %ld\n", fres - beg);
                auto s = std::string(beg, fres);
                printf("s = %s\n", s.data());
                auto len = std::stoll(s, nullptr, 16);
                printf("len %lld\n", len);
                if(len == 0) {
                    // printf("Request finished\n");
                    break;
                }
                beg = fres + 2;
                auto maximum = buffer.size() - (end - buffer.begin());
                auto need = len - (end - beg);
                if(need + 2 > maximum) {
                    buffer.resize(buffer.size() + need + 2 - maximum);
                    printf("Todo recreate iterators\n");
                    abort();
                }
                if(!in.ReceiveAll(buffer.data() + (end - buffer.begin()), need + 2)) {
                    ok = false;
                    // printf("Failed to receive all data\n");
                }
                output << std::string(beg, end + need);
            } else {
                // output << std::string(buffer.begin(), buffer.begin() + len);
                //std::cout << std::string(beg, end) << "\n";
            }
            
        }
    }
// #ifndef NDEBUG
//     printf("http response body: %s\n", output.str().c_str());
// #endif
    SSL_CTX_free(sslctx);
    // abort();
    return http_response(ok, status, output.str());
}

void http_request::perform(std::function<void(http_response)> success, std::function<void(std::exception_ptr)> error) {
    success(perform());
}