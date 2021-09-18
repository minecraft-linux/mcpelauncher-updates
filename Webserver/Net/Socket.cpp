#include "Socket.h"
#include <stdexcept>
#include <algorithm>
#include <cstring>

using namespace Net;

Socket::Socket(Socket && socket)
{
#ifdef _WIN32
	{
		WSADATA wsaData;
		WSAStartup(MAKEWORD(2,2), &wsaData);
	}
#endif
	handle = socket.handle;
	socketaddress = socket.socketaddress;
	socket.handle = -1;
}

Socket::Socket(SOCKET handle, const std::shared_ptr<sockaddr> & socketaddress)
{
#ifdef _WIN32
	{
		WSADATA wsaData;
		WSAStartup(MAKEWORD(2,2), &wsaData);
	}
#endif
	this->handle = handle;
	this->socketaddress = socketaddress;
}

Socket::~Socket() {
	if (handle != -1) {
		shutdown(handle, 2);
		closesocket(handle);
		handle = -1;
	}
#ifdef _WIN32
	WSACleanup();
#endif
}

std::shared_ptr<Socket> Socket::Connect(std::string address, short port) {
#ifdef _WIN32
	{
		WSADATA wsaData;
		WSAStartup(MAKEWORD(2,2), &wsaData);
	}
#endif
 	struct addrinfo hints, *result, *ptr;
	memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

	if(getaddrinfo(address.data(), std::to_string(port).data(), &hints, &result) == 0) {
		for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {
			auto sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
			if(sock != -1) {
				if(connect(sock, ptr->ai_addr, (int)ptr->ai_addrlen) != -1) {
					auto socketaddress = std::shared_ptr<sockaddr>((sockaddr*)new char[ptr->ai_addrlen]);
					freeaddrinfo(result);
					auto psock = std::make_shared<Socket>(sock, socketaddress);
#ifdef _WIN32
					WSACleanup();
#endif
					return psock;
				}
			}
		}
		freeaddrinfo(result);
	}
#ifdef _WIN32
	WSACleanup();
#endif
	return nullptr;
}


Socket::SocketOutputStream Socket::GetOutputStream() {
	return Socket::SocketOutputStream(*this, std::unique_lock<std::mutex>(writelock));
}
Socket::SocketInputStream Socket::GetInputStream() {
	return Socket::SocketInputStream(*this, std::unique_lock<std::mutex>(readlock));
}

SOCKET Socket::GetHandle() {
	return handle;
}

std::string Socket::GetAddress() {
	char buf[255];
	return std::string(inet_ntop(socketaddress->sa_family, socketaddress.get(), buf, sizeof(buf)));
}

uint16_t Socket::GetPort() {
	if (socketaddress->sa_family == AF_INET) {
		return ntohs((*(sockaddr_in*)socketaddress.get()).sin_port);
	}
	else if (socketaddress->sa_family == AF_INET6) {
		return ntohs((*(sockaddr_in6*)socketaddress.get()).sin6_port);
	}
	return -1;
}

void Socket::SetProtocol(const std::string & protocol) {
	this->protocol = protocol;
}

std::string Socket::GetProtocol() {
	return protocol;
}

int Socket::Receive(uint8_t * buffer, int length) {
	return recv(handle, (char*)buffer, length, 0);
}

Net::Socket::SocketInputStream::SocketInputStream(Net::Socket &handle, std::unique_lock<std::mutex> &&lock) : handle(handle), lock(std::move(lock)) {

}

size_t Socket::SocketInputStream::Receive(uint8_t * buffer, size_t length) {
	return handle.Receive(buffer, length <= std::numeric_limits<int>::max() ? (int)length : std::numeric_limits<int>::max());
}

bool Socket::SocketInputStream::ReceiveAll(uint8_t * buffer, size_t length) {
	uint8_t * end = buffer + length;
	while (buffer < end) {
		int received = handle.Receive(buffer, (end - buffer) <= std::numeric_limits<int>::max() ? (int)(end - buffer) : std::numeric_limits<int>::max());
		if (received <= 0) {
			return false;
		}
		buffer += received;
	}
	return true;
}

int Socket::Send(const uint8_t * buffer, int length) {
	return send(handle, (const char*)buffer, length, 0);
}

Net::Socket::SocketOutputStream::SocketOutputStream(Net::Socket &handle, std::unique_lock<std::mutex> &&lock) : handle(handle), lock(std::move(lock)) {
	
}

size_t Socket::SocketOutputStream::Send(const uint8_t * buffer, size_t length) {
	const uint8_t * end = buffer + length;
	while (buffer < end) {
		int sent = handle.Send(buffer, (end - buffer) <= std::numeric_limits<int>::max() ? (int)(end - buffer) : std::numeric_limits<int>::max());
		if (sent <= 0) {
			return length - (end - buffer);
		}
		buffer += sent;
	}
	return length;
}

size_t Socket::SocketOutputStream::Send(std::vector<uint8_t> buffer) {
	return Send(buffer.data(), buffer.size());
}

size_t Socket::SocketOutputStream::Send(std::vector<uint8_t> buffer, size_t length) {
	return Send(buffer.data(), length);
}

bool Socket::SocketOutputStream::SendAll(const uint8_t * buffer, size_t length) {
	return Send(buffer, length) == length;
}

bool Socket::SocketOutputStream::SendAll(std::vector<uint8_t> buffer) {
	return SendAll(buffer.data(), buffer.size());
}

bool Socket::SocketOutputStream::SendAll(std::vector<uint8_t> buffer, size_t length) {
	return SendAll(buffer.data(), length);
}

int Socket::SendTo(uint8_t * buffer, int length, const sockaddr * to, socklen_t tolength) {
	return sendto(handle, (char*)buffer, length, 0, to, tolength);
}

int Socket::ReceiveFrom(uint8_t * buffer, int length, sockaddr * from, socklen_t * fromlength) {
	return recvfrom(handle, (char*)buffer, length, 0, from, fromlength);
}
