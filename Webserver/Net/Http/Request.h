#pragma once
#include "../Socket.h"
#include "Header.h"	
#include "V2/HPack/Decoder.h"
#include "Header.h"
#include <string>
#include <memory>
#include <unordered_map>
#include <cstdint>

namespace Net
{
	namespace Http
	{
		class Request : public Header
		{
		public:
			Request(const std::shared_ptr<HeaderImpl>& headerimpl);
			void ParseUri(const std::string &path);
			std::string method;
			std::string uri;
			std::string path;
			std::string query;
		};
	}
}