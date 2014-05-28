#include <algorithm>
#include <cassert>

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

	bytes_inserted(0),

	recent_min(0),
	recent_max(0),

	nr(0),

	state(State::Filling),

	buffer(fifo_size)
{
	assert(fifo_size >= fifo_high_watermark);
	assert(fifo_high_watermark >= fifo_low_watermark);
	assert(fifo_size > 0);
}

void ClientQueue::insert(EM::Data input, uint nr)
{
	if (input.length > get_available_space_size() || nr <= this->nr || input.length == 0) {
		std::cerr << "Package dropped!\n";
		return;
	}
	buffer.write(input);

	update_recent_data();

	bytes_inserted += input.length;

	if (get_size() >= fifo_high_watermark)
		state = State::Active;

	this->nr = nr;
}

EM::Data ClientQueue::get(size_t length)
{
	return buffer.read(length);
}

void ClientQueue::move(size_t length)
{
	buffer.move(length);

	if (get_size() <= fifo_low_watermark)
		state = State::Filling;
}

bool ClientQueue::is_full() const
{
	return get_size() == fifo_size;
}

void ClientQueue::clear()
{
	bytes_inserted = 0;
	recent_min     = 0;
	recent_max     = 0;
	buffer.clear();
	buffer.init();
}

size_t ClientQueue::get_size() const
{
	return buffer.get_size();
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

bool ClientQueue::is_active() const
{
	return state == State::Active;
}

bool ClientQueue::is_filling() const
{
	return state == State::Filling;
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

bool ClientObject::is_active() const
{
	return queue.is_active();
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
		queue.get_max_size(), queue.get_min_recent_bytes(), queue.get_max_recent_bytes());

	return std::string(report);
}

void ClientObject::set_udp_endpoint(boost::asio::ip::udp::endpoint udp_endpoint)
{
	this->udp_endpoint = udp_endpoint;
}

boost::asio::ip::udp::endpoint ClientObject::get_udp_endpoint()
{
	return udp_endpoint;
}
