#include "Android.h"
#include <vector>
#include "../../../Http/TLSSocketListener.h"

void * Initialize()
{
	//OPENSSL_init();
	return new Net::TLSSocketListener();
}

char * Start(void * obj, int port, void(*callback)(const char * ip, int port), _Files(*getFilenames)(), _File(*getFile)(const char * name))
{
	char * res;
	try {
		(*(Net::TCPSocketListener *)obj).SetConnectionHandler([callback](std::shared_ptr<Net::TCPSocket> socket) {
			std::vector<uint8_t> buffer;
			const in6_addr &in6addr = socket->GetAddress();
			if (IN6_IS_ADDR_V4MAPPED(&in6addr))
			{
				buffer.resize(INET_ADDRSTRLEN);
				in_addr inaddr;
				inaddr.s_addr = in6addr.in6_u.u6_addr32[3];
				inet_ntop(AF_INET, &inaddr, (char*)buffer.data(), sizeof(inaddr));
			}
			else
			{
				buffer.resize(INET6_ADDRSTRLEN);
				inet_ntop(AF_INET6, &in6addr, (char*)buffer.data(), sizeof(in6addr));
			}
			callback((const char*)buffer.data(), socket->GetPort());
			buffer.resize(10240);
			int read = socket->Receive(buffer.data(), buffer.size());
		});
		(*(Net::TCPSocketListener *)obj).Listen(in6addr_any, port);
		res = new char[7];
		memcpy(res, "Sucess", 7);
	}
	catch (const std::exception & ex) {
		auto msg = ex.what();
		size_t len = strlen(msg) + 1;
		res = new char[len];
		memcpy(res, msg, len);
		return (char *)ex.what();
	}
	return res;
}

void Cancel(void * obj)
{
	(*(Net::TCPSocketListener *)obj).Cancel();
}

void Destroy(void * obj)
{
	//OPENSSL_free(nullptr);
	if (obj != nullptr) delete (Net::TCPSocketListener *)obj;
}
