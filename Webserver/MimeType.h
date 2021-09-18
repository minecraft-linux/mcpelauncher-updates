#pragma once
#include <string>
#include <unordered_map>

using namespace std;

class MimeType
{
public:
	string Get(const string & extention);
};