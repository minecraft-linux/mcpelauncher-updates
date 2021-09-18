#pragma once
#include "Header.h"
#include <sstream>
#include <string>
#include <vector>
#include <memory>

namespace Net
{
	namespace Http
	{
		class Response : public Header
		{
		public:
			Response(const std::shared_ptr<HeaderImpl>& headerimpl);
			int status;
			virtual bool Add(size_t hash, const std::pair<std::string, std::string> & pair) override;
		};
	}
}