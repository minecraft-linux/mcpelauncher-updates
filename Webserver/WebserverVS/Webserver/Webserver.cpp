#include "../../Net/TLSSocketListener.h"
#include "../../Net/Http/V2/Frame.h"
#include "../../Net/Http/V2/HPack/Encoder.h"
#include "../../Net/Http/V2/HPack/Decoder.h"
#include "../../Net/Http/V2/Setting.h"
#include "../../Net/Http/V2/Stream.h"
#include "../../Net/Http/V2/ErrorCode.h"
#include "../../Net/Http/V2/Connection.h"
#include "../../Net/Http/V1/Connection.h"
#include "../../Net/Http/V2/Session.h"
//#include "../PHPSapi/PHPSapi.h"

#include <unordered_map>
#include <algorithm>
#include <vector>
#include <memory>
#include <fstream>
#include <iostream>
#include <cstdint>

#include <experimental/filesystem>
using namespace Net::Http;
using namespace std::experimental::filesystem;

//void RequestHandler(std::shared_ptr<Connection> connection)
//{
//	auto & request = connection->request;
//	auto & response = connection->response;
//	std::vector<uint8_t> buffer;
//	if (request.method == "GET")
//	{
//		if (request.path == "/status.html")
//		{
//			auto & socket = connection->socket;
//			response.status = 200;
//			response.headerlist.insert({ "content-length", "23" });
//			connection->SendResponse();
//			connection->SendData((uint8_t*)"Http/1-2 Server Running", 23, true);
//		}
//		else
//		{
//			path filepath = L"D:\\Web" / request.path;
//			if (is_regular_file(filepath))
//			{
//				if (filepath.extension() == ".php")
//				{
//					PHPSapi::requesthandler(connection);
//				}
//				else
//				{
//					uintmax_t filesize = file_size(filepath);
//					response.status = 200;
//					response.headerlist.insert({ "content-length", std::to_string(filesize) });
//					connection->SendResponse();
//					{
//						std::vector<uint8_t> buffer(10240);
//						std::ifstream filestream(filepath, std::ios::binary);
//						for (uintmax_t i = filesize; i > 0;)
//						{
//							int count = std::min((uintmax_t)buffer.size(), i);
//							filestream.read((char*)buffer.data(), count);
//							connection->SendData(buffer.data(), count, count == i);
//							i -= count;
//						}
//					}
//				}
//			}
//		}
//	}
//	else
//	{
//		if (request.path == "/upload")
//		{
//			connection->SetOnData([connection](std::vector<uint8_t>::const_iterator & buffer, uint32_t length) {
//				std::cout << "-------------------------------\n" << std::string((char*)&buffer[0], length) << "-------------------------------\n";
//				connection->request.contentlength -= length;
//				if (connection->request.contentlength == 0)
//				{
//					connection->response.status = 200;
//					connection->response.headerlist.insert({ "content-length", "0" });
//					connection->SendResponse();
//				}
//			});
//		}
//	}
//}

int main(int argc, const char** argv)
{
	Net::TLSSocketListener listener;
	listener.SetConnectionHandler([](std::shared_ptr<Net::Socket> socket) {
		if (socket->GetProtocol() == "h2")
		{
			using namespace V2;
			try
			{
				auto session = std::make_shared<V2::Session>(socket);
				session->Start();
			}
			catch (const std::runtime_error & error)
			{
				std::cout << error.what() << "\n";
			}
		}
		//else
		//{
		//	int content = 0;
		//	std::shared_ptr<V1::Connection> connection;
		//	while (true)
		//	{
		//		int count = socket->Receive(buffer.data(), content == 0 ? buffer.size() : std::min(content, (int)buffer.size()));
		//		if (count > 0)
		//		{
		//			if (content <= 0)
		//			{
		//				connection = std::make_shared<V1::Connection>();
		//				connection->request.DecodeHttp1(buffer.begin(), buffer.end());
		//				connection->socket = socket;
		//				content = connection->request.contentlength;
		//				RequestHandler(connection);
		//			}
		//			else
		//			{
		//				content -= count;
		//				connection->OnData(buffer.begin(), count);
		//			}
		//		}
		//		else
		//		{
		//			break;
		//		}
		//	}
		//}
	});
	listener.UsePrivateKey("D:\\Users\\administrator\\Documents\\privatekey.pem", Net::SSLFileType::PEM);
	listener.UseCertificate("D:\\Users\\administrator\\Documents\\certificate.cer", Net::SSLFileType::PEM);
	auto address = std::make_shared<sockaddr_in6>();
	memset(address.get(), 0, sizeof(sockaddr_in6));
	address->sin6_family = AF_INET6;
	address->sin6_port = htons(443);
	address->sin6_addr = in6addr_any;
	listener.AddProtocol("h2");
	//PHPSapi::init();
	listener.Listen(std::shared_ptr<sockaddr>(address, (sockaddr*)address.get()), sizeof(sockaddr_in6))->join();
	//PHPSapi::deinit();
	return 0;
}