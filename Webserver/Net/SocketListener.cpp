#include "SocketListener.h"
#include <stdexcept>
#include <string>
#ifndef _WIN32
#include <netinet/tcp.h>
#endif

using namespace Net;

SocketListener::SocketListener()
{
#ifdef _WIN32
	WSADATA data;
	WSAStartup(WINSOCK_VERSION, &data);
#endif // _WIN32
	handle = -1;
	//clients = 0;
}

SocketListener::~SocketListener()
{
	Cancel();
#ifdef _WIN32
	WSACleanup();
#endif // _WIN32
}

bool SocketListener::OnConnection(std::shared_ptr<Socket> socket)
{
	if (!_onconnection/* || clients > 10*/)
		return false;
	//clients++;
	std::thread([this, socket]() {
		_onconnection(socket);
		//clients--;
	}).detach();
	return true;
}

std::shared_ptr<std::thread> Net::SocketListener::Listen(const std::shared_ptr<sockaddr>& address, socklen_t addresslen)
{
	if (handle == -1)
	{
		if ((handle = socket(address->sa_family, SOCK_STREAM, 0)) == -1)
			return listener = nullptr;
		uint32_t value = 0;
		setsockopt(handle, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&value, sizeof(value));
		value = 1;
		setsockopt(handle, IPPROTO_TCP, TCP_FASTOPEN, (const char*)&value, sizeof(value));
		setsockopt(handle, SOL_SOCKET, SO_REUSEADDR, (const char*)&value, sizeof(value));
	}
	if (bind(handle, address.get(), addresslen) == -1)
	{
		return nullptr;
	}
	if (listen(handle, 10) == -1)
	{
		return nullptr;
	}
	cancel = false;
	return listener = std::make_shared<std::thread>([this]() {
		while (!cancel)
		{
			auto socket = Accept();
			if (socket)
				OnConnection(socket);
		}
	});
}

void SocketListener::Cancel()
{
	cancel = true;
	if (handle != -1)
	{
		shutdown(handle, 2);
		closesocket(handle);
		handle = -1;
	}
	if (listener && listener->joinable())
		listener->join();
}

std::shared_ptr<Socket> SocketListener::Accept()
{
	std::shared_ptr<sockaddr_storage> address = std::make_shared<sockaddr_storage>();
	socklen_t size = sizeof(sockaddr_storage);
	SOCKET socket = accept(this->handle, (sockaddr*)address.get(), &size);
	if (socket == -1)
		return std::shared_ptr<Socket>();
	return std::make_shared<Socket>(socket, std::shared_ptr<sockaddr>(address, (sockaddr*)address.get()));
}

void SocketListener::SetConnectionHandler(std::function<void(std::shared_ptr<Socket>)> onconnection)
{
	_onconnection = onconnection;
}
