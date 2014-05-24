#ifndef EMSERVER_H
#define EMSERVER_H

#include <queue>
#include <boost/asio.hpp>
#include <sys/types.h>
#include <unordered_map>

#include "Server/ClientObject.h"
#include "Server/TcpConnection.h"
#include "System/AbstractServer.h"
#include "System/Mixer.h"

class EMServer : public AbstractServer
{
public:
	EMServer();
	~EMServer();

	void set_port(uint port);
	uint get_port() const;

	void set_fifo_size(uint fifo_size);
	uint get_fifo_size() const;
	void set_fifo_low_watermark(uint fifo_low_watermark);
	uint get_fifo_low_watermark() const;
	void set_fifo_high_watermark(uint fifo_high_watermark);
	uint get_fifo_high_watermark() const;

	void set_buffer_length(uint buffer_length);
	uint get_buffer_length() const;

	void set_tx_interval(uint tx_interval);
	uint get_tx_interval() const;

	void start();
	void quit();

	virtual uint get_next_cid();
	virtual void add_client(uint cid);
	virtual void on_connection_established(uint cid, Connection *connection);
	virtual void on_connection_lost(uint cid);

private:
	void start_accept();
	void handle_accept(TcpConnection::Pointer new_connection,
	                   const boost::system::error_code &error);
	void send_info_routine();

	uint port;

	uint fifo_size;
	uint fifo_low_watermark;
	uint fifo_high_watermark;

	uint buffer_length;

	uint tx_interval;

	//TODO make it thread safe
	std::unordered_map<uint, ClientObject *> clients;

	boost::asio::io_service io_service;
	boost::asio::ip::tcp::acceptor *tcp_acceptor;
};

#endif // EMSERVER_H
