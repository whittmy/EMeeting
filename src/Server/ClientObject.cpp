#include <algorithm>

#include "Server/ClientObject.h"
#include "System/Messages.h"

/**
 * \class ClientQueue
 */

ClientQueue::ClientQueue(
	size_t fifo_size,
	size_t fifo_low_watermark,
	size_t fifo_high_watermark) :

	fifo_size(fifo_size),
	fifo_low_watermark(fifo_low_watermark),
	fifo_high_watermark(fifo_high_watermark),

	current_size(0),
	bytes_inserted(0)
{}

void ClientQueue::push(EM::MixerInput input)
{
	queue.push(input);

	if (get_size() > fifo_high_watermark)
		state = State::Active;

	update_recent_data();

	size_t bytes = sizeof(input);

	bytes_inserted += bytes;
	current_size   += bytes;
}

EM::MixerInput ClientQueue::pop()
{
	EM::MixerInput input = front();
	queue.pop();

	if (get_size() < fifo_low_watermark)
		state = State::Filling;

	update_recent_data();

	current_size -= sizeof(input);

	return input;
}

EM::MixerInput ClientQueue::front()
{
	return queue.front();
}

bool ClientQueue::is_full() const
{
	return get_size() == fifo_size;
}

size_t ClientQueue::get_size() const
{
	return current_size;
}

size_t ClientQueue::get_max_size() const
{
	return fifo_size;
}

size_t ClientQueue::get_bytes_inserted() const
{
	return bytes_inserted;
}

size_t ClientQueue::get_min_recent_bytes() const
{
	return recent_min;
}

size_t ClientQueue::get_max_recent_bytes() const
{
	return recent_max;
}

void ClientQueue::reset_recent_data()
{
	recent_min = recent_max = get_size();
}

ClientQueue::State ClientQueue::get_current_state() const
{
	return state;
}

void ClientQueue::update_recent_data()
{
	recent_min = std::min(recent_min, get_size());
	recent_max = std::max(recent_max, get_size());
}

/**
 * \class ClientObject
 */

ClientObject::ClientObject(
	uint cid,
	size_t fifo_size,
	size_t fifo_low_watermark,
	size_t fifo_high_watermark) :

	cid(cid),
	queue(fifo_size, fifo_low_watermark, fifo_high_watermark),

	connection(nullptr)
{}

uint ClientObject::get_cid() const
{
	return cid;
}

ClientQueue &ClientObject::get_queue()
{
	return queue;
}

std::string ClientObject::get_name() const
{
	return connection->get_name();
}

void ClientObject::set_connection(TcpConnection *connection)
{
	this->connection = connection;
}

TcpConnection *ClientObject::get_connection()
{
	return connection;
}

bool ClientObject::is_connected() const
{
	return connection != nullptr;
}

std::string ClientObject::get_report() const
{
	static const size_t BUFFER_SIZE = 128;
	char report[BUFFER_SIZE];

	std::sprintf(report, EM::Messages::List.c_str(), get_name().c_str(), queue.get_size(),
	             queue.get_max_size());

	return std::string(report);
}
