#ifndef EMSERVER_H
#define EMSERVER_H

#include <queue>
#include <boost/asio.hpp>
#include <sys/types.h>
#include <unordered_map>

#include "System/AbstractServer.h"
#include "System/Mixer.h"
#include "System/TcpConnection.h"

class ClientQueue
{
public:
	ClientQueue(uint fifo_size, uint fifo_low_watermark, uint fifo_high_watermark);

	enum class State : uint8_t {Filling, Active};

	void push(EM::MixerInput input);
	EM::MixerInput pop();
	EM::MixerInput front();
	bool full() const;
	size_t size() const;

	State get_current_state() const;

private:
	uint fifo_size;
	uint fifo_low_watermark;
	uint fifo_high_watermark;

	std::queue<EM::MixerInput> queue;

	State state;
};

class ClientObject
{
public:
	ClientObject(uint cid, uint fifo_size, uint fifo_low_watermark, uint fifo_high_watermark);

	uint get_cid() const;
	ClientQueue &get_queue();

	void set_connected(bool connected);
	bool is_connected() const;

private:
	uint cid;
	ClientQueue queue;
	bool connected;
};

class EMServer : public AbstractServer
{
public:
	EMServer(boost::asio::io_service &io_service);
	~EMServer();

	void set_port(uint port);
	uint get_port() const;

	void set_fifo_size(int fifo_size);
	uint get_fifo_size() const;
	void set_fifo_low_watermark(int fifo_low_watermark);
	uint get_fifo_low_watermark() const;
	void set_fifo_high_watermark(int fifo_high_watermark);
	uint get_fifo_high_watermark() const;

	void set_buffer_length(uint buffer_length);
	uint get_buffer_length() const;

	void set_tx_interval(uint tx_interval);
	uint get_tx_interval() const;

	void start();
	void quit();

	void add_client(uint cid);
	void on_connection_established(uint cid);
	void on_connection_lost(uint cid);

private:
	void start_accept();
	void handle_accept(TcpConnection::Pointer new_connection,
	                   const boost::system::error_code &error);

	uint port;

	uint fifo_size;
	uint fifo_low_watermark;
	uint fifo_high_watermark;

	uint buffer_length;

	uint tx_interval;

	std::unordered_map<uint, ClientObject *> clients;

	boost::asio::io_service &io_service;
	boost::asio::ip::tcp::acceptor *tcp_acceptor;
};

#endif // EMSERVER_H
