#ifndef MESSAGES_H
#define MESSAGES_H

#include <string>
#include <sys/types.h>

namespace EM {
	std::string client(uint client_id);
	std::string clients(const std::string &client,
	                    size_t current_fifo_size,
	                    size_t max_fifo_size,
	                    size_t min_fifo_size_registered,
	                    size_t max_fifo_size_registered);

	/** These datagrams precede data stream */
	std::string upload(uint nr);
	std::string data(uint nr, uint ack, size_t win);

	std::string ack(uint ack, size_t win);
	std::string retransmit(uint nr);
	std::string keep_alive();
}

#endif // MESSAGES_H
