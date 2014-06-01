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

		Type get_type(const std::string &str);
		Type get_type(const char *str, size_t length);

		bool read_client(const std::string &message, uint &nr);

		bool read_data(
			const std::string &message,
			uint &nr,
			uint &ack,
			size_t &win);

		bool read_ack(const std::string &message, uint &ack, size_t &win);

		bool read_upload(const std::string &message, uint &nr);

		bool read_retransmit(const std::string &message, uint &nr);
	}
}

#endif // MESSAGES_H
