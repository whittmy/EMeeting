#ifndef MESSAGES_H
#define MESSAGES_H

#include <string>
#include <sys/types.h>

namespace EM {
	namespace Messages {
		const std::string Client     = "CLIENT %u\n";
		const std::string List       = "%s FIFO: %u/%u (min. %u, max. %u)\n";
		const std::string Upload     = "UPLOAD %u\n";
		const std::string Data       = "DATA %u %u %u\n";
		const std::string Ack        = "ACK %u %u\n";
		const std::string Retransmit = "RETRANSMIT %u\n";
		const std::string KeepAlive  = "KEEPALIVE\n";
	}
}

#endif // MESSAGES_H
