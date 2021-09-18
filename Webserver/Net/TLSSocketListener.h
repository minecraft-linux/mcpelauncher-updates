#pragma once
#include "SocketListener.h"
#include <cstdint>
#include <memory>
#include <string>
#include <openssl/ssl.h>

namespace Net
{
	enum class SSLFileType
	{
		PEM = SSL_FILETYPE_PEM,
		DER = X509_FILETYPE_ASN1
	};

	class TLSSocketListener : public SocketListener
	{
	private:
		SSL_CTX * sslctx;
		std::string protocols;
	protected:
		std::shared_ptr<Socket> Accept() override;
	public:
		TLSSocketListener();
		~TLSSocketListener();
		void AddProtocol(const std::string &proto);
		const std::string & GetProtocols();
		bool UsePrivateKey(const std::string & privatekey, SSLFileType ftype);
		bool UsePrivateKey(const uint8_t * buffer, int length, SSLFileType ftype);
		bool UseCertificate(const std::string & certificate, SSLFileType ftype);
		bool UseCertificate(const uint8_t * buffer, int length, SSLFileType ftype);
		std::shared_ptr<std::thread> Listen(const std::shared_ptr<sockaddr> &address, socklen_t addresslen) override;
	};
}