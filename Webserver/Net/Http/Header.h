#pragma once
#include <cstdint>
#include <string>
#include <math.h>
#include <unordered_map>
#include <memory>
#include <vector>
#include "HeaderImpl.h"

namespace Net
{
	namespace Http
	{
		class Header
		{
		protected:
			std::shared_ptr<HeaderImpl> headerimpl;
			Header(const std::shared_ptr<HeaderImpl>& headerimpl);
		public:
			static std::hash<std::string> hash_fn;
			std::string scheme;
			std::string authority;
			std::string contenttype;
			uintmax_t contentlength;
			std::unordered_multimap<std::string, std::string> headerlist;
			void Add(const std::pair<std::string, std::string> & pair);
			virtual bool Add(size_t hash, const std::pair<std::string, std::string> & pair);
			void Encode(std::vector<uint8_t>::iterator & buffer) const;
			void Decode(std::vector<uint8_t>::const_iterator & buffer, const std::vector<uint8_t>::const_iterator & end);
		};
	}
}