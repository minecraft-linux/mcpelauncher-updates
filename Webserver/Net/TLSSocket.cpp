#include "TLSSocket.h"
#include <vector>

using namespace Net;

TLSSocket::TLSSocket(SSL * ssl, intptr_t socket, const std::shared_ptr<sockaddr> &socketaddress) : Socket(socket, socketaddress)
{
	this->ssl = ssl;
}

TLSSocket::TLSSocket(SSL * ssl, const std::shared_ptr<Socket> &socket) : Socket(std::move(*socket))
{
	this->ssl = ssl;
}

TLSSocket::~TLSSocket()
{
	SSL_shutdown(ssl);
	SSL_free(ssl);
}

std::shared_ptr<Net::TLSSocket> TLSSocket::Connect(std::string address, short port, bool verify) {
	std::shared_ptr<Net::Socket> sock = Socket::Connect(address, port);
	if(!sock) return nullptr;
	SSL_CTX * ctx = SSL_CTX_new(TLS_client_method());
	SSL * ssl = SSL_new(ctx);
	SSL_CTX_free(ctx);
	int ret = SSL_set_fd(ssl, sock->GetHandle());
	if(!verify) {
		SSL_set_verify(ssl, SSL_VERIFY_NONE, nullptr);
	}
	ret = SSL_connect(ssl);
	if(ret != 1) {
		SSL_free(ssl);
		return nullptr;
	}
	return std::make_shared<Net::TLSSocket>(ssl, sock);
}


int TLSSocket::Receive(uint8_t * buffer, int length)
{
	return SSL_read(ssl, buffer, length);
}

int TLSSocket::Send(const uint8_t * buffer, int length)
{
	return SSL_write(ssl, buffer, length);
}
