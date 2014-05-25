#include <cstdio>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

#include "Server/TcpConnection.h"
#include "System/Messages.h"

TcpConnection::~TcpConnection()
{
	server->on_connection_lost(cid);
}

TcpConnection::Pointer TcpConnection::create(
	AbstractServer *server,
	boost::asio::io_service &io_service)
{
	return Pointer(new TcpConnection(server, io_service));
}

void TcpConnection::start()
{
	std::cerr << "Starting connection...\n";
	char msg[128];

	cid = server->get_next_cid();
	std::sprintf(msg, EM::Messages::Client.c_str(), cid);

	std::string message(msg, std::strlen(msg));

	std::cerr << "Sending message: " << message;

	boost::asio::async_write(socket, boost::asio::buffer(message),
		boost::bind(&TcpConnection::handle_connect, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}

void TcpConnection::send_info(const std::string &info)
{
	boost::asio::async_write(socket, boost::asio::buffer(info),
		boost::bind(&TcpConnection::handle_write, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}

boost::asio::ip::tcp::socket &TcpConnection::get_socket()
{
	return socket;
}

uint TcpConnection::get_cid() const
{
	return cid;
}

std::string TcpConnection::get_name() const
{
	std::string name;
	try {
		name = boost::lexical_cast<std::string>(socket.remote_endpoint());
	} catch (boost::bad_lexical_cast) {
		return std::string("unknown");
	}
	return name;
}

TcpConnection::TcpConnection(AbstractServer *server, boost::asio::io_service &io_service) :
	server(server), socket(io_service)
{}

void TcpConnection::handle_connect(const boost::system::error_code &error, size_t size)
{
	if (error)
		std::cerr << "handle_connect: error\n";
	else
		server->on_connection_established(cid, this);
}

void TcpConnection::handle_write(const boost::system::error_code &error, size_t size)
{
	if (error) {
		std::cerr << "handle_write: error\n";
		server->on_connection_lost(cid);
	}
}
