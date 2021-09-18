#include "MimeType.h"

using namespace std;

unordered_map<string, string> MimeTypeTable = {
	{ u8".html" , u8"text/html; charset=utf-8" },
	{ u8".css" , u8"text/css; charset=utf-8" },
	{ u8".js", u8"text/javascript; charset=utf-8" },
	{ u8".xml" , u8"text/xml; charset=utf-8" },
	{ u8".json" , u8"text/json; charset=utf-8" },
	{ u8".svg", u8"text/svg; charset=utf-8" },
	{ u8".txt" , u8"text/plain; charset=utf-8" },
	{ u8".png" , u8"image/png" },
	{ u8".jpg" , u8"image/jpeg" },
	{ u8".tiff" , u8"image/tiff" },
	{ u8".fif", u8"image/fif" },
	{ u8".ief" , u8"image/ief" },
	{ u8".gif", u8"image/gif" },
	{ u8".pdf", u8"application/pdf" },
	{ u8".mpg", u8"video/mpeg" },
	{ u8".wasm", u8"application/wasm" },
};

string MimeType::Get(const string & extention)
{
	auto result = MimeTypeTable.find(extention);
	return result == MimeTypeTable.end() ? u8"application/octet-stream" : result->second;
}
