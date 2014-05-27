#include "System/Messages.h"

EM::Messages::Type EM::Messages::get_type(const std::string &str)
{
	std::string s;
	std::stringstream ss(str);

	ss >> s;
	if (header_to_type.find(s) != header_to_type.end())
		return header_to_type.at(s);
	else
		return Type::Unknown;
}

EM::Messages::Type EM::Messages::get_type(const char *str, size_t length)
{
	return get_type(std::string(str, length));
}

bool EM::Messages::read_data(const char *buffer, size_t length, uint &nr, uint &ack, size_t &win)
{
	std::string s;
	std::stringstream ss(std::string(buffer, length));

	ss >> s;
	if (s != Headers::Data)
		return false;

	ss >> nr >> ack >> win;

	return !ss.bad();
}

bool EM::Messages::read_ack(const char *buffer, size_t length, uint &ack, size_t &win)
{
	std::string s;
	std::stringstream ss(std::string(buffer, length));

	ss >> s;
	if (s != Headers::Ack)
		return false;

	ss >> ack >> win;

	return !ss.bad();
}