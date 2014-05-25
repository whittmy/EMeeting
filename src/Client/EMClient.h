#ifndef EMCLIENT_H
#define EMCLIENT_H

#include <boost/asio.hpp>
#include <condition_variable>
#include <mutex>
#include <string>
#include <unordered_map>

#include "System/Mixer.h"

class EMClient
{
public:
	EMClient(std::istream &in, std::ostream &out);
	~EMClient();

	void set_port(uint port);
	uint get_port() const;
	void set_server_name(const std::string server_name);
	std::string get_server_name() const;

	void set_retransmit_limit(uint retransmit_limit);
	uint get_retransmit_limit() const;

	void start();
	void quit();

private:
	bool connect_tcp();
	bool read_init_message();

	void connect_udp();
	void send_data_to_server();
	void read_data_from_server();

	void keep_alive_routine();
	bool ask_retransmit(uint number);
	bool send_datagram(void *data, size_t length, uint number);
	bool wait_for_ack();

	std::istream &in;
	std::ostream &out;

	uint port;
	std::string server_name;

	uint cid;

	uint retransmit_limit;

	boost::asio::io_service io_service;

	boost::asio::ip::tcp::socket tcp_socket;
	boost::asio::ip::tcp::resolver tcp_resolver;

	boost::asio::ip::udp::socket udp_socket;
	boost::asio::ip::udp::resolver udp_resolver;
	boost::asio::ip::udp::endpoint udp_endpoint;

	static const size_t BUFFER_SIZE = 16777216;
	static const size_t MSG_SIZE    = 4194304;

	static const uint KEEP_ALIVE_FREQUENCY = 500;

	std::mutex data_mutex;

	/** what server expects */
	uint datagram_client_server_number;
	/** what was sent */
	uint datagram_client_sent_number;
	/** what came from input stream */
	uint datagram_client_input_number;

	std::mutex datagram_client_mutex;
	std::condition_variable datagram_client_cond;

	size_t window_size;

	/** expected by the client */
	uint datagram_server_client_number;
};

#endif // EMCLIENT_H

