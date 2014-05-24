#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "System/AbstractServer.h"

class TcpConnection : public boost::enable_shared_from_this<TcpConnection>
{
public:
	typedef boost::shared_ptr<TcpConnection> Pointer;

	static Pointer create(AbstractServer *server, boost::asio::io_service &io_service);
	void start();

	boost::asio::ip::tcp::socket &get_socket();

private:
	TcpConnection(AbstractServer *server, boost::asio::io_service &io_service);

	void handle_write(const boost::system::error_code &error, size_t size);

	AbstractServer *server;
	boost::asio::ip::tcp::socket socket;
};

#endif // TCPCONNECTION_H
