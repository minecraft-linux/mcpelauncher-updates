#include "../../Net/Socket.h"
#include <iostream>
#include <string>
#include <cstdint>
#include <fstream>

using namespace Net;

int main(int argc, char * * argv)
{
	WSADATA d;
	WSAStartup(WINSOCK_VERSION, &d);
	//Socket sock;
	SOCKET handle = socket(AF_INET, SOCK_DGRAM, 0);
	if (argc > 1)
	{
		ip_mreq mreq;
		mreq.imr_interface.S_un.S_addr = INADDR_ANY;
		inet_pton(AF_INET, "224.1.2.15", &mreq.imr_multiaddr);
		uint32_t value = 1;
		int r = setsockopt(handle, SOL_SOCKET, SO_REUSEADDR, (const char*)&value, sizeof(value));
		r = setsockopt(handle, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char*)&mreq, sizeof(mreq));
		uint8_t buffer[512];
		sockaddr_in addr;
		memset(&addr, 0, sizeof(sockaddr_in));
		addr.sin_family = AF_INET;
		addr.sin_addr = mreq.imr_interface;
		addr.sin_port = 8192;
		int fromlen = sizeof(addr);
		r = bind(handle, (sockaddr*)&addr, fromlen);
		size_t length = 0;
		size_t i = 0;
		std::ofstream file("tmp.apk");
		do
		{
			int recved = recvfrom(handle, (char*)buffer, 512, 0, 0, 0);
			if(recved != 512)
				throw std::runtime_error("!!!!!");
			uint32_t plength = ntohl(*(uint32_t*)buffer);
			if (i == 0)
				length = plength;
			if (plength != length)
				throw std::runtime_error("!!!!!");
			uint32_t poffset = ntohl(*((uint32_t*)buffer + 1));
			if (poffset != i)
				throw std::runtime_error("!!!!!");
			size_t end = i + 504;
			file.write((char*)buffer + 8, end > length ? length - i : 504);
			i = end;
		} while (i < length);
		
		//std::cout << std::string((char*)buffer, r);
	}
	else
	{
		sockaddr_in saddr;
		memset(&saddr, 0, sizeof(sockaddr_in));
		saddr.sin_family = AF_INET;
		inet_pton(AF_INET, "224.1.2.15", &saddr.sin_addr);
		saddr.sin_port = 8192;
		std::ifstream ifstream("C:\\Users\\Christopher\\Desktop\\Webserver.Android.Webserver.Android.apk");
		ifstream.seekg(0, std::ios::end);
		size_t length = ifstream.tellg();
		ifstream.seekg(0);
		uint8_t buf[512];
		*(uint32_t*)buf = htonl(length);
		for (size_t i = 0; i < length;)
		{
			*((uint32_t*)buf + 1) = htonl(i);
			size_t end = i + 504;
			ifstream.read((char*)buf + 8, end > length ? length - i : 504);
			size_t sent = sendto(handle, (char*)buf, 512, 0, (const sockaddr*)&saddr, sizeof(saddr));
			if (sent != 512)
				throw std::runtime_error("!!!!!");
			i = end;
		}
	}
	closesocket(handle);
	WSACleanup();
}