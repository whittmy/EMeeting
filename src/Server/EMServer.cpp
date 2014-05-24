#include <boost/bind.hpp>
#include <iostream>

#include "Server/EMServer.h"
#include "System/Messages.h"
#include "System/Utils.h"
#include "System/TcpConnection.h"

using namespace boost::asio;

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
 * \class ClientObject
 */

ClientObject::ClientObject(
	uint cid,
	uint fifo_size,
	uint fifo_low_watermark,
	uint fifo_high_watermark) :

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

void ClientObject::set_connected(bool connected)
{
	this->connected = connected;
}

bool ClientObject::is_connected() const
{
	return connected;
}

/**
 * \class EMServer
 */

EMServer::EMServer(boost::asio::io_service &io_service) :
	AbstractServer(),

	port(EM::Default::PORT),

	fifo_size(EM::Default::FIFO_SIZE),
	fifo_low_watermark(EM::Default::FIFO_LOW_WATERMARK),
	fifo_high_watermark(EM::Default::FIFO_HIGH_WATERMARK),

	buffer_length(EM::Default::BUFFER_LENGTH),

	tx_interval(EM::Default::TX_INTERVAL),

	io_service(io_service)
{}

EMServer::~EMServer()
{
	quit();
}

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
	tcp_acceptor = new ip::tcp::acceptor(io_service, ip::tcp::endpoint(ip::tcp::v4(), port));
	std::cerr << "accepting connections on port " << port << " (IPv4)\n";
	start_accept();
}

void EMServer::quit()
{
	for (auto obj : clients)
		delete obj.second;
}

void EMServer::add_client(uint cid)
{
	clients[cid] =
		new ClientObject(cid,
		                 get_fifo_size(),
		                 get_fifo_low_watermark(),
		                 get_fifo_high_watermark());
}

void EMServer::on_connection_established(uint cid)
{
	clients[cid]->set_connected(true);
}

void EMServer::on_connection_lost(uint cid)
{
	clients[cid]->set_connected(false);
}

void EMServer::start_accept()
{
	TcpConnection::Pointer new_connection =
		TcpConnection::create(this, tcp_acceptor->get_io_service());
	std::cerr << "waiting for connections...\n";
	tcp_acceptor->async_accept(
		new_connection->get_socket(),
		boost::bind(&EMServer::handle_accept, this, new_connection,
		            boost::asio::placeholders::error));
}

void EMServer::handle_accept(
	TcpConnection::Pointer new_connection,
        const boost::system::error_code &error)
{
	if (!error)
		new_connection->start();
	else
		std::cerr << "handle_accept: error\n";
	start_accept();
}