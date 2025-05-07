#include "webserv.hpp"

std::string tostr(const int value)
{
	std::stringstream ss;
	ss << value;
	return ss.str();
}

//template <typename T>
//bool strto_safe(const std::string& str, T& result)
//{
//    std::stringstream ss(str);
//    ss >> result;
//    return !ss.fail() && ss.eof();
//}
