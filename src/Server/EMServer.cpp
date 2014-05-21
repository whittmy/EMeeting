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

/**
 * \class ClientQueue
 */

ClientQueue::ClientQueue(uint fifo_size, uint fifo_low_watermark, uint fifo_high_watermark) :
	fifo_size(fifo_size),
	fifo_low_watermark(fifo_low_watermark),
	fifo_high_watermark(fifo_high_watermark)
{}

void ClientQueue::push(EM::mixer_input input)
{
	queue.push(input);

	if (sizeof(EM::mixer_input) * queue.size() > fifo_high_watermark)
		state = State::Active;
}

EM::mixer_input ClientQueue::pop()
{
	EM::mixer_input input = front();
	queue.pop();

	if (sizeof(EM::mixer_input) * queue.size() < fifo_low_watermark)
		state = State::Filling;

	return input;

}

EM::mixer_input ClientQueue::front()
{
	return queue.front();
}

bool ClientQueue::full() const
{
	return queue.size() == fifo_size;
}

ClientQueue::State ClientQueue::get_current_state() const
{
	return state;
}

/**
 * \class EMServer
 */

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

uint EMServer::get_port() const
{
	return port;
}

void EMServer::set_fifo_size(int fifo_size)
{
	this->fifo_size = fifo_size;
}

uint EMServer::get_fifo_size() const
{
	return fifo_size;
}

void EMServer::set_fifo_low_watermark(int fifo_low_watermark)
{
	this->fifo_low_watermark = fifo_low_watermark;
}

uint EMServer::get_fifo_low_watermark() const
{
	return fifo_low_watermark;
}

void EMServer::set_fifo_high_watermark(int fifo_high_watermark)
{
	this->fifo_high_watermark = fifo_high_watermark;
}

uint EMServer::get_fifo_high_watermark() const
{
	return fifo_high_watermark;
}

void EMServer::set_buffer_length(uint buffer_length)
{
	this->buffer_length = buffer_length;
}

uint EMServer::get_buffer_length() const
{
	return buffer_length;
}

void EMServer::set_tx_interval(uint tx_interval)
{
	this->tx_interval = tx_interval;
}

uint EMServer::get_tx_interval() const
{
	return tx_interval;
}

void EMServer::start()
{

}
