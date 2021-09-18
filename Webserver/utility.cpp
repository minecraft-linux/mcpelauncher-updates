//http://bogomip.net/blog/cpp-url-encoding-and-decoding/
#include "utility.h"
 
#include <sstream>
#include <stdexcept>
#include <iomanip>
#include <cstring>

namespace Utility {
	std::string Replace(const std::string &str, const char * search, const char * replace)
	{
		std::string res = str;
		for (size_t pos = res.find(search); pos < res.length() && pos != std::string::npos; pos = res.find(search, pos))
		{
			res = res.replace(pos, strlen(search), replace);
			pos += strlen(replace);
		}
		return res;
	}

    std::string UrlEncode(const std::string &toEncode) {
        std::ostringstream out;
         
        for(std::string::size_type i=0; i < toEncode.length(); ++i) {
            short t = toEncode.at(i);
             
            if(
                (t >= 45 && t <= 46) ||       // hyphen-period
                (t >= 48 && t <= 57) ||       // 0-9
                (t >= 65 && t <= 90) ||       // A-Z
                t == 95 ||          // underscore
                (t >= 97 && t <= 122) ||  // a-z
                t == 126            // tilde
            ) {
                out << toEncode.at(i);
            } else {
                out << CharToHex(toEncode.at(i));
            }
        }
         
        return out.str();
    }
 
    std::string UrlDecode(const std::string &toDecode) {
        std::ostringstream out;
         
        for(std::string::size_type i=0; i < toDecode.length(); ++i) {
            if(toDecode.at(i) == '%') {
                std::string str(toDecode.substr(i+1, 2));
                out << HexToChar(str);
                i += 2;
            } else {
                out << toDecode.at(i);
            }
        }
         
        return out.str();
    }
 
    std::string CharToHex(unsigned char c) {
        short i = c;
         
        std::ostringstream s;
         
        s << "%" << std::setw(2) << std::setfill('0') << std::hex << i;
         
        return s.str();
    }
 
    unsigned char HexToChar(const std::string &str) {
        short c = 0;
         
        if(!str.empty()) {
            std::istringstream in(str);
             
            in >> std::hex >> c;
             
            if(in.fail()) {
                throw std::runtime_error("stream decode failure");
            }
        }
         
        return static_cast<unsigned char>(c);
    }
}