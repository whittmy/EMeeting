#include "EMServer.h"

namespace EM {
	namespace Default {
		static const uint NR_ALB = 334695;

		namespace Server {
			static const uint PORT = 10000 + NR_ALB % 10000;

			static const uint FIFO_SIZE           = 10560;
			static const uint FIFO_LOW_WATERMARK  = 0;
			static const uint FIFO_HIGH_WATERMARK = FIFO_SIZE;

			static const uint BUFFER_LENGTH    = 10;

			static const uint TX_INTERVAL = 5;
		}
	}
}

EMServer::EMServer() :
	port(EM::Default::Server::PORT),

	fifo_size(EM::Default::Server::FIFO_SIZE),
	fifo_low_watermark(EM::Default::Server::FIFO_LOW_WATERMARK),
	fifo_high_watermark(EM::Default::Server::FIFO_HIGH_WATERMARK),

	buffer_length(EM::Default::Server::BUFFER_LENGTH),

	tx_interval(EM::Default::Server::TX_INTERVAL)
{}

void EMServer::set_port(uint port)
{
	this->port = port;
}

void EMServer::set_fifo_size(int fifo_size)
{
	this->fifo_size = fifo_size;
}

void EMServer::set_fifo_low_watermark(int fifo_low_watermark)
{
	this->fifo_low_watermark = fifo_low_watermark;
}

void EMServer::set_fifo_high_watermark(int fifo_high_watermark)
{
	this->fifo_high_watermark = fifo_high_watermark;
}

void EMServer::set_buffer_length(uint buffer_length)
{
	this->buffer_length = buffer_length;
}

void EMServer::set_tx_interval(uint tx_interval)
{
	this->tx_interval = tx_interval;
}

