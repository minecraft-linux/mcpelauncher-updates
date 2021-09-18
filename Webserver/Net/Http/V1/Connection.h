#pragma once
#include "../Connection.h"

namespace Net
{
	namespace Http
	{
        namespace V1
        {
            class Connection : public Net::Http::Connection
            {
                Request request;
				Response response;
                std::shared_ptr<Socket> socket;
            public:
                Connection(const std::shared_ptr<Socket>& socket);
                virtual Request& GetRequest() override;
				virtual Response& GetResponse() override;
                virtual void SendResponse(bool endstream = false) override;
                virtual void SendData(const uint8_t * buffer, int length, bool endstream = false) override;
            };
        }
    }
}