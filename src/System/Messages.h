#ifndef MESSAGES_H
#define MESSAGES_H

#include <cctype>
#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>

namespace EM {
	namespace Messages {
		namespace Headers {
			const std::string Client     = "CLIENT";
			const std::string Upload     = "UPLOAD";
			const std::string Data       = "DATA";
			const std::string Ack        = "ACK";
			const std::string Retransmit = "RETRANSMIT";
			const std::string KeepAlive  = "KEEPALIVE";
		}

		const std::string Client     = Headers::Client + " %u\n";
		const std::string List       = "%s FIFO: %u/%u (min. %u, max. %u)\n";
		const std::string Upload     = Headers::Upload + " %u\n";
		const std::string Data       = Headers::Data + " %u %u %u\n";
		const std::string Ack        = Headers::Ack + " %u %u\n";
		const std::string Retransmit = Headers::Retransmit + " %u\n";
		const std::string KeepAlive  = Headers::KeepAlive + "\n";

		enum class Type : uint8_t {
			Client,
			List,
			Upload,
			Data,
			Ack,
			Retransmit,
			KeepAlive,
			Unknown,
		};

		const std::unordered_map<std::string, Type> header_to_type {
			{Headers::Client,     Type::Client},
			{Headers::Upload,     Type::Upload},
			{Headers::Data,       Type::Data},
			{Headers::Ack,        Type::Ack},
			{Headers::Retransmit, Type::Retransmit},
			{Headers::KeepAlive,  Type::KeepAlive},
		};

		const size_t LENGTH = 128;

		static Type get_type(const std::string &str);
		static Type get_type(const char *str, size_t length);

		static bool read_data(
			const char *buffer,
			size_t length,
			uint &nr,
			uint &ack,
			size_t &win);

		static bool read_ack(const char *buffer, size_t length, uint &ack, size_t &win);
	}
}

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

	std::cerr << "read " << ack << " and " << win << "\n";

	return !ss.bad();
}

#endif // MESSAGES_H
