#include "Http.h"

std::string Net::Http::V1::KeyUpper(const std::string & source)
{
	std::string result(source);
	bool upper = true;
	for (auto & ch : result)
	{
		if (upper)
		{
			ch = toupper(ch);
		}
		upper = !isalpha(ch);
	}
	return result;
}
