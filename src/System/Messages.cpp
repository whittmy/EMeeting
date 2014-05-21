#include "Messages.h"

std::string EM::client(uint client_id)
{
	return std::string("CLIENT ") + std::to_string(client_id) + std::string("\n");
}

std::string EM::clients(const std::string &client,
                        size_t current_fifo_size,
                        size_t max_fifo_size,
                        size_t min_fifo_size_registered,
                        size_t max_fifo_size_registered)
{
	return client + std::string(" FIFO: ") + std::to_string(current_fifo_size) +
	       std::string("/") + std::to_string(max_fifo_size) + std::string(" (min. ") +
	       std::to_string(min_fifo_size_registered) + std::string(", max. ") +
	       std::to_string(max_fifo_size_registered) + std::string(")\n");
}

std::string EM::upload(uint nr)
{
	return std::string("UPLOAD ") + std::to_string(nr) + std::string("\n");
}

std::string EM::data(uint nr, uint ack, size_t win)
{
	return std::string("DATA ") + std::to_string(nr) + std::string(" ") +
	       std::to_string(ack) + std::string(" ") + std::to_string(win) + std::string("\n");
}

std::string EM::ack(uint ack, size_t win)
{
	return std::string("ACK ") + std::to_string(ack) + std::string(" ") + std::to_string(win) +
	       std::string("\n");
}

std::string EM::retransmit(uint nr)
{
	return std::string("RETRANSMIT ") + std::to_string(nr) + std::string("\n");
}

std::string EM::keep_alive()
{
	return std::string("KEEPALIVE\n");
}

