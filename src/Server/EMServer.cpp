#include <iostream>

#include "Server/EMServer.h"
#include "System/Messages.h"
#include "System/Utils.h"

/**
 * \class ClientQueue
 */

ClientQueue::ClientQueue(uint fifo_size, uint fifo_low_watermark, uint fifo_high_watermark) :
	fifo_size(fifo_size),
	fifo_low_watermark(fifo_low_watermark),
	fifo_high_watermark(fifo_high_watermark)
{}

void ClientQueue::push(EM::MixerInput input)
{
	queue.push(input);

	if (size() > fifo_high_watermark)
		state = State::Active;
}

EM::MixerInput ClientQueue::pop()
{
	EM::MixerInput input = front();
	queue.pop();

	if (size() < fifo_low_watermark)
		state = State::Filling;

	return input;
}

EM::MixerInput ClientQueue::front()
{
	return queue.front();
}

bool ClientQueue::full() const
{
	return queue.size() == fifo_size;
}

size_t ClientQueue::size() const
{
	return queue.size() * sizeof(EM::MixerInput);
}

ClientQueue::State ClientQueue::get_current_state() const
{
	return state;
}

/**
 * \class EMServer
 */

EMServer::EMServer() :
	port(EM::Default::PORT),

	fifo_size(EM::Default::FIFO_SIZE),
	fifo_low_watermark(EM::Default::FIFO_LOW_WATERMARK),
	fifo_high_watermark(EM::Default::FIFO_HIGH_WATERMARK),

	buffer_length(EM::Default::BUFFER_LENGTH),

	tx_interval(EM::Default::TX_INTERVAL)
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
	while (true) {
	}
}

void EMServer::quit()
{
}