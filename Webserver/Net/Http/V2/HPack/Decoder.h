#pragma once
#include <cstdint>
#include <vector>
#include <string_view>
#include <sstream>
#include <algorithm>
#include "../ErrorCode.h"
#include "../../Header.h"
#include "HPack.h"

namespace Net
{
	namespace Http
	{
		namespace V2
		{
			namespace HPack
			{
				class Decoder
				{
				private:
					std::vector<std::pair<std::string, std::string>> dynamictable;

					template<class Iter>
					static Iter DecodeInteger(Iter pos, uint8_t bits, uint64_t& number) {
						uint8_t mask = (1 << bits) - 1;
						number = *pos & mask;
						if (number == mask)
						{
							uint64_t pbits = 0;
							do
							{
								number += (*++pos & 127) << pbits;
								pbits += 7;
							} while ((*pos & 128) == 128);
						}
						return ++pos;
					}

					template<class Iter>
					static Iter DecodeHuffmanString(Iter pos, ptrdiff_t length, std::string& str) {
						std::ostringstream strs;
						long long  i = 0, blength = length << 3;
						union { 
							uint64_t data64;
							uint32_t data32[2];
							uint8_t data8[8];
						} buffer;
						auto end = pos + length;
						while (true)
						{
							{
								uint32_t count = std::min<uint32_t>(5, (uint32_t)(end - pos));
								std::reverse_copy(pos, pos + count, (buffer.data8) + (5 - count));
							}
							const std::pair<uint32_t, uint8_t> *res = std::find_if(StaticHuffmanTable, StaticHuffmanTable + 256, [buffer = (uint32_t)(buffer.data64 >> (8 - (i % 8)))](const std::pair<uint32_t, uint8_t> & entry) -> bool {
								return entry.first == (buffer >> (32 - entry.second));
							});
							pos += (i % 8 + res->second) >> 3;
							if (((i += res->second) > blength) || (res >= (StaticHuffmanTable + 255)))
							{
								str = strs.str();
								return end;
							}
							strs << (char)(res - StaticHuffmanTable);
						}
					}

					template<class Iter>
					static Iter DecodeString(Iter pos, std::string& str) {
						bool huffmanencoding = (*pos & 0x80) == 0x80;
						uint64_t length;
						pos = DecodeInteger(pos, 7, length);
						if (huffmanencoding)
						{
							return DecodeHuffmanString(pos, length, str);
						}
						else
						{
							str = std::string(pos, pos + length);
							return pos += length;
						}
					}
				public:
					void DecodeHeaderblock(Net::Http::Header * request, std::vector<uint8_t>::const_iterator & pos, const std::vector<uint8_t>::const_iterator & end)
					{
						while (pos != end)
						{
							if ((*pos & 0x80) != 0)
							{
								uint64_t index;
								pos = DecodeInteger(pos, 7, index);
								if (index >= (62 + dynamictable.size()) || index == 0)
									throw Error(Error::Type::Connection, Error::Code::COMPRESSION_ERROR, "Invalid Index: " + std::to_string(index));
								request->Add(index < 62 ? std::pair<std::string, std::string>(StaticTable[index - 1]) : *(dynamictable.end() - (ptrdiff_t)(index - 61)));
							}
							else if ((*pos & 0x40) != 0)
							{
								uint64_t index;
								pos = DecodeInteger(pos, 6, index);
								std::string key, value;
								if (index == 0)
								{
									pos = DecodeString(pos, key);
								}
								else
								{
									if (index > (62 + dynamictable.size()) || index == 0)
										throw Error(Error::Type::Connection, Error::Code::COMPRESSION_ERROR, "Invalid Index: " + std::to_string(index));
									key = (index < 62 ? HPack::StaticTable[index - 1] : *(dynamictable.end() - (ptrdiff_t)(index - 61))).first;
								}
								if (key.empty())
								{
									throw Error(Error::Type::Connection, Error::Code::COMPRESSION_ERROR);
								}
								pos = DecodeString(pos, value);
								if (value.empty())
									throw Error(Error::Type::Connection, Error::Code::COMPRESSION_ERROR);
								request->Add({ key, value });
								dynamictable.push_back({ key, value });
							}
							else if ((*pos & 0x20) != 0) {
								uint64_t maxsize;
								pos = DecodeInteger(pos, 5, maxsize);
							}
							else if ((*pos & 0x10) != 0) {
								uint64_t index;
								pos = DecodeInteger(pos, 4, index);
								std::string key, value;
								if (index == 0) {
									pos = DecodeString(pos, key);
								}
								else {
									if (index > (62 + dynamictable.size()) || index == 0)
										throw Error(Error::Type::Connection, Error::Code::COMPRESSION_ERROR, "Invalid Index: " + std::to_string(index));
									key = (index < 62 ? HPack::StaticTable[index - 1] : *(dynamictable.end() - (ptrdiff_t)(index - 61))).first;
								}
								if (key.empty())
									throw Error(Error::Type::Connection, Error::Code::COMPRESSION_ERROR);
								pos = DecodeString(pos, value);
								if (value.empty())
									throw Error(Error::Type::Connection, Error::Code::COMPRESSION_ERROR);
								request->Add({ key, value });
							}
							else
							{
								uint64_t index;
								pos = DecodeInteger(pos, 4, index);
								std::string key, value;
								if (index == 0)
								{
									pos = DecodeString(pos, key);
								}
								else
								{
									if (index > (62 + dynamictable.size()) || index == 0)
										throw Error(Error::Type::Connection, Error::Code::COMPRESSION_ERROR, "Invalid Index: " + std::to_string(index));
									key = (index < 62 ? HPack::StaticTable[index - 1] : *(dynamictable.end() - (ptrdiff_t)(index - 61))).first;
								}
								if (key.empty())
								{
									throw Error(Error::Type::Connection, Error::Code::COMPRESSION_ERROR);
								}
								pos = DecodeString(pos, value);
								if (value.empty())
								{
									throw Error(Error::Type::Connection, Error::Code::COMPRESSION_ERROR);
								}
								request->Add({ key, value });
							}
							if (pos > end)
							{
								throw Error(Error::Type::Connection, Error::Code::COMPRESSION_ERROR);
							}
						}
					}
				};
			}
		}
	}
}