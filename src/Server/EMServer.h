#ifndef EMSERVER_H
#define EMSERVER_H

#include <queue>
#include <boost/asio.hpp>
#include <mutex>
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
	uint get_connected_clients_number() const;
	uint get_active_clients_number() const;

	uint get_cid_from_address(const std::string &address);

	void udp_receive_routine();
	void handle_receive(const boost::system::error_code &ec, size_t bytes_received);
	void send_ack(uint nr, size_t win);
	void send_data(uint cid, uint nr, uint ack, size_t win, EM::Data data);

	void mixer_routine();

	uint port;

	uint fifo_size;
	uint fifo_low_watermark;
	uint fifo_high_watermark;

	uint buffer_length;

	uint tx_interval;

	mutable std::mutex clients_mutex;
	std::unordered_map<uint, ClientObject *> clients;

	boost::asio::io_service io_service;
	boost::asio::ip::tcp::acceptor *tcp_acceptor;

	boost::asio::ip::udp::socket udp_socket;
	boost::asio::ip::udp::endpoint udp_endpoint;

	static const size_t BUFFER_SIZE            = 65536;
	static const size_t EXPECTED_CLIENTS_LIMIT = 16;

	char buffer[BUFFER_SIZE];

	DataBuffer mixer_buffer;
};

#endif // EMSERVER_H
