#pragma once
#include "../Socket.h"
#include <memory>

namespace Net {
	namespace Http {
		class Session : public std::enable_shared_from_this<Session>
		{
		protected:
			std::shared_ptr<Net::Socket> socket;
		public:
			Session(std::shared_ptr<Net::Socket> &socket) : socket(socket){

			}
		};
	}
}