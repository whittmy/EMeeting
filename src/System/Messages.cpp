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

bool EM::Messages::read_client(const std::string &str, uint &nr)
{
	std::string s;
	std::stringstream ss(str);

	ss >> s;
	if (s != Headers::Client)
		return false;

	ss >> nr;

	return !ss.bad();
}


bool EM::Messages::read_data(const std::string &message, uint &nr, uint &ack, size_t &win)
{
	std::string s;
	std::stringstream ss(message);

	ss >> s;
	if (s != Headers::Data)
		return false;

	ss >> nr >> ack >> win;

	return !ss.bad();
}

bool EM::Messages::read_ack(const std::string &message, uint &ack, size_t &win)
{
	std::string s;
	std::stringstream ss(message);

	ss >> s;
	if (s != Headers::Ack)
		return false;

	ss >> ack >> win;

	return !ss.bad();
}

bool EM::Messages::read_upload(const std::string &message, uint &nr)
{
	std::string s;
	std::stringstream ss(message);

	ss >> s;
	if (s != Headers::Upload)
		return false;

	ss >> nr;

	return !ss.bad();
}

bool EM::Messages::read_retransmit(const std::string &message, uint &nr)
{
	std::string s;
	std::stringstream ss(message);

	ss >> s;
	if (s != Headers::Upload)
		return false;

	ss >> nr;

	return !ss.bad();
}
