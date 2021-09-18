#include "../Connection.h"
#include "Stream.h"
#include "HPack/Encoder.h"

namespace Net
{
	namespace Http
	{
		namespace V2
		{
			class Connection : public Net::Http::Connection
			{
				Request request;
				Response response;
				std::shared_ptr<Stream> stream;
			public:
				Connection(const std::shared_ptr<Stream>& stream, const std::shared_ptr<HPack::Decoder> &decoder, const std::shared_ptr<HPack::Encoder> &encoder);
				virtual Request& GetRequest() override;
				virtual Response& GetResponse() override;
				virtual void SendResponse(bool endstream = false) override;
				virtual void SendData(const uint8_t * buffer, int length, bool endstream = false) override;
			};
		}
    }
}