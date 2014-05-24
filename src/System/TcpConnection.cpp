#include <cstdio>
#include <boost/bind.hpp>

#include "System/Messages.h"
#include "System/TcpConnection.h"

TcpConnection::Pointer TcpConnection::create(
	AbstractServer *server,
	boost::asio::io_service &io_service)
{
	return Pointer(new TcpConnection(server, io_service));
}

void TcpConnection::start()
{
	std::cerr << "starting connection...\n";
	char msg[100];
	std::sprintf(msg, EM::Messages::Client.c_str(), server->get_next_cid());

	std::string message(msg);
	boost::asio::async_write(socket, boost::asio::buffer(message),
		boost::bind(&TcpConnection::handle_write, shared_from_this(),
		boost::asio::placeholders::error,
		boost::asio::placeholders::bytes_transferred));
}

boost::asio::ip::tcp::socket &TcpConnection::get_socket()
{
	return socket;
}

TcpConnection::TcpConnection(AbstractServer *server, boost::asio::io_service &io_service) :
	server(server), socket(io_service)
{}

void TcpConnection::handle_write(const boost::system::error_code &error, size_t size)
{
	if (error) {
		std::cerr << "handle_write: error\n";
	}
}
