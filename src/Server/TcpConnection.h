#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "System/AbstractServer.h"

class TcpConnection : public boost::enable_shared_from_this<TcpConnection>, public Connection
{
public:
	TcpConnection(AbstractServer *server, boost::asio::io_service &io_service);
	virtual ~TcpConnection();

	typedef boost::shared_ptr<TcpConnection> Pointer;

	static Pointer create(AbstractServer *server, boost::asio::io_service &io_service);
	void start();
	void send_info(const std::string &info);

	boost::asio::ip::tcp::socket &get_socket();

	uint get_cid() const;

	virtual std::string get_name() const;

private:

	void handle_connect(const boost::system::error_code &error, size_t size);
	void handle_write(const boost::system::error_code &error, size_t size);

	AbstractServer *server;
	boost::asio::ip::tcp::socket socket;

	uint cid;
};

#endif // TCPCONNECTION_H
