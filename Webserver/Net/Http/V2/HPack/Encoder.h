#pragma once
#include "HPack.h"
#include <algorithm>
#include <vector>
#include <deque>
#include <string>
#include <utility>

namespace Net
{
	namespace Http
	{
		namespace V2
		{
			namespace HPack
			{
				class Encoder
				{
				private:
					std::deque<std::pair<std::string, std::string>> dynamictable;
				public:
					template<class Iter> Iter AddInteger(Iter buffer, uint64_t integer, uint8_t head, uint8_t bits)
					{
						*buffer = head;
						uint8_t mask = (1 << bits) - 1;
						if (integer < mask)
						{
							*buffer++ |= integer;
						}
						else
						{
							*buffer++ |= mask;
							integer -= mask;
							while (integer >= 0x80)
							{
								*buffer++ = (uint8_t)((integer % 0x80) | 0x80);
								integer >>= 7;
							}
							*buffer++ = (uint8_t)integer;
						}
						return buffer;
					}

					template<class Iter1, class Iter2> Iter1 AddString(Iter1 buffer, const Iter2 & beg, const Iter2 & end)
					{
						buffer = AddInteger(buffer, end - beg, 0, 7);
						buffer = std::copy(beg, end, buffer);
						return buffer;
					}

					template<class Iter, class T> void AddHeaderBlock(Iter & buffer, const T& headerlist)
					{
						std::hash<std::string_view> hash_fn;
						std::hash<std::string> hash_fn2;
						for (const std::pair<std::string, std::string> & entry : headerlist) {
							auto pred = [&hash_fn, hash = hash_fn(entry.first)](const std::pair<std::string_view, std::string_view> & pair) {
								return hash == hash_fn(pair.first);
							};
							auto pred2 = [&hash_fn2, hash = hash_fn2(entry.first)](const std::pair<std::string, std::string> & pair) {
								return hash == hash_fn2(pair.first);
							};
							auto tentry = std::find_if(StaticTable, std::end(StaticTable), pred);
							if (tentry != std::end(StaticTable))
							{
								auto eentry = std::find(tentry, std::end(StaticTable), entry);
								if (eentry != std::end(StaticTable))
								{
									buffer = AddInteger(buffer, (eentry - StaticTable) + 1, 0x80, 7);
								}
								else
								{
									buffer = AddInteger(buffer, (tentry - StaticTable) + 1, 0, 4);
									buffer = AddString(buffer, entry.second.begin(), entry.second.end());
								}
							}
							else
							{
								auto res = std::find_if(dynamictable.begin(), dynamictable.end(), pred2);
								if (res != dynamictable.end())
								{
									auto eentry = std::find(res, dynamictable.end(), entry);
									if (eentry != dynamictable.end())
									{
										buffer = AddInteger(buffer, (eentry - dynamictable.begin()) + 1, 0x80, 7);
									}
									else
									{
										buffer = AddInteger(buffer, (res - dynamictable.begin()) + 62, 0, 4);
										buffer = AddString(buffer, entry.second.begin(), entry.second.end());
									}
								}
								else
								{
									buffer = AddInteger(buffer, 0, 0x40, 6);
									buffer = AddString(buffer, entry.first.begin(), entry.first.end());
									buffer = AddString(buffer, entry.second.begin(), entry.second.end());
									dynamictable.push_front(entry);
								}
							}
						}
					}
				};
			}
		}
	}
}