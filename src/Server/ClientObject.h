#ifndef CLIENTOBJECT_H
#define CLIENTOBJECT_H

#include <cctype>
#include <string>

#include "Server/TcpConnection.h"

class ClientQueue
{
public:
	ClientQueue(size_t fifo_size, size_t fifo_low_watermark, size_t fifo_high_watermark);

	bool insert(const std::string &data, uint nr);
	std::string get(size_t length);
	bool move(size_t length);
	bool is_full() const;
	void clear();

	size_t get_size() const;
	size_t get_max_size() const;
	size_t get_available_space_size() const;
	size_t get_bytes_inserted() const;

	size_t get_min_recent_bytes() const;
	size_t get_max_recent_bytes() const;
	void reset_recent_data();

	uint get_expected_nr() const;

	enum class State : uint8_t {Active, Filling};

	State get_state() const;
	bool is_active() const;
	bool is_filling() const;

private:
	void update_recent_data();

	size_t fifo_size;
	size_t fifo_low_watermark;
	size_t fifo_high_watermark;

	size_t bytes_inserted;

	size_t recent_min;
	size_t recent_max;

	uint nr;

	State state;

	std::string buffer;
};

class ClientObject
{
public:
	ClientObject(
		uint cid,
		size_t fifo_size,
		size_t fifo_low_watermark,
		size_t fifo_high_watermark);

	uint get_cid() const;
	ClientQueue &get_queue();

	bool is_active() const;

	std::string get_name() const;

	void set_connection(TcpConnection::Pointer connection);
	TcpConnection::Pointer get_connection();
	bool is_connected() const;

	std::string get_report() const;

	void set_udp_endpoint(boost::asio::ip::udp::endpoint udp_endpoint);
	boost::asio::ip::udp::endpoint get_udp_endpoint();

private:
	uint cid;
	ClientQueue queue;

	TcpConnection::Pointer connection;
	boost::asio::ip::udp::endpoint udp_endpoint;
};

#endif // CLIENTOBJECT_H
