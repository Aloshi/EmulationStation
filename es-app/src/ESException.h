#pragma once

#include <exception>
#include <sstream>

class ESException : public std::exception
{
public:
	const char* what() const throw() override
	{
		return mMsg.c_str();
	}

	template<typename T>
	friend ESException& operator<<(ESException& e, T msg);
private:
	std::string mMsg;
};

template<typename T>
ESException& operator<<(ESException& e, T appendMsg)
{
	std::stringstream ss;
	ss << e.mMsg << appendMsg;
	e.mMsg = ss.str();
	return e;
}