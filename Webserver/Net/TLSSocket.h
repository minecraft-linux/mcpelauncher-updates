#pragma once
#include "Socket.h"
#include <openssl/ssl.h>

namespace Net
{
	class TLSSocket : public Socket
	{
	private:
		SSL * ssl;
	protected:
		int Receive(uint8_t * buffer, int length) override;
		int Send(const uint8_t * buffer, int length) override;
	public:
		TLSSocket(SSL * ssl, intptr_t socket, const std::shared_ptr<sockaddr> &socketaddress);
		TLSSocket(SSL * ssl, const std::shared_ptr<Socket>& socket);
		static std::shared_ptr<TLSSocket> Connect(const std::string address, short port, bool verify);
		virtual ~TLSSocket();
	};
}