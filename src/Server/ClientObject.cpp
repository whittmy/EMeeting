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
	bytes_inserted(0),

	recent_min(0),
	recent_max(0),

	nr(0)
{}

void ClientQueue::push(ClientQueue::Data input, uint nr)
{
	if (get_size() + input.length > get_max_size() || nr <= this->nr) {
		std::cerr << "Package dropped!\n";
		return;
	}
	queue.push(input);

	std::cerr << "input from client: " << input.data << " (" << input.length << ")\n";

	if (get_size() > fifo_high_watermark)
		state = State::Active;

	update_recent_data();

	bytes_inserted += input.length;
	current_size   += input.length;

	this->nr = nr;
}

ClientQueue::Data ClientQueue::pop()
{
	Data input = front();
	queue.pop();

	if (get_size() < fifo_low_watermark)
		state = State::Filling;

	update_recent_data();

	current_size -= input.length;

	return input;
}

ClientQueue::Data ClientQueue::front()
{
	return queue.front();
}

bool ClientQueue::is_full() const
{
	return get_size() == fifo_size;
}

void ClientQueue::clear()
{
	current_size   = 0;
	bytes_inserted = 0;
	recent_min     = 0;
	recent_max     = 0;
	while (!queue.empty()) {
		ClientQueue::Data d = pop();
		delete d.data;
	}
}

size_t ClientQueue::get_size() const
{
	return current_size;
}

size_t ClientQueue::get_max_size() const
{
	return fifo_size;
}

size_t ClientQueue::get_available_space_size() const
{
	return get_max_size() - get_size();
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

uint ClientQueue::get_expected_nr() const
{
	return nr + 1;
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
	queue(fifo_size, fifo_low_watermark, fifo_high_watermark)
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
	if (connection == nullptr)
		return std::string("unknown");
	else
		return connection->get_name();
}

void ClientObject::set_connection(TcpConnection::Pointer connection)
{
	this->connection = connection;
}

TcpConnection::Pointer ClientObject::get_connection()
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
