#ifndef MESSAGES_H
#define MESSAGES_H

#include <cctype>
#include <cstdarg>
#include <cstring>
#include <iostream>
#include <string>

namespace EM {
	namespace Messages {
		const std::string Client     = "CLIENT %u\n";
		const std::string List       = "%s FIFO: %u/%u (min. %u, max. %u)\n";
		const std::string Upload     = "UPLOAD %u\n";
		const std::string Data       = "DATA %u %u %u\n";
		const std::string Ack        = "ACK %u %u\n";
		const std::string Retransmit = "RETRANSMIT %u\n";
		const std::string KeepAlive  = "KEEPALIVE\n";

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

		const size_t LENGTH = 128;

		static Type get_type(const std::string &str);
		static Type get_type(const char *str);

	}
}

EM::Messages::Type EM::Messages::get_type(const std::string &str)
{
	std::string s;
	uint arg1 = 0, arg2 = 0, arg3 = 0, arg4 = 0;

	if (std::sscanf(str.c_str(), EM::Messages::Client.c_str(), &arg1) == 1)
		return EM::Messages::Type::Client;
	else if (std::sscanf(
		str.c_str(), EM::Messages::List.c_str(), &s[0], &arg1, &arg2, &arg3, &arg4)
			== 5)
		return EM::Messages::Type::List;
	else if (std::sscanf(str.c_str(), EM::Messages::Upload.c_str(), &arg1) == 1)
		return EM::Messages::Type::Upload;
	else if (std::sscanf(str.c_str(), EM::Messages::Data.c_str(), &arg1, &arg2, &arg3) == 3)
		return EM::Messages::Type::Data;
	else if (std::sscanf(str.c_str(), EM::Messages::Ack.c_str(), &arg1, &arg2) == 2)
		return EM::Messages::Type::Ack;
	else if (std::sscanf(str.c_str(), EM::Messages::Retransmit.c_str(), &arg1) == 1)
		return EM::Messages::Type::Retransmit;
	else if (std::strcmp(str.c_str(), EM::Messages::KeepAlive.c_str()) == 0)
		return EM::Messages::Type::KeepAlive;
	else
		return EM::Messages::Type::Unknown;
}

EM::Messages::Type EM::Messages::get_type(const char *str)
{
	return get_type(std::string(str, EM::Messages::LENGTH));
}

#endif // MESSAGES_H
