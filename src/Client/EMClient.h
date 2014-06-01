#ifndef EMCLIENT_H
#define EMCLIENT_H

#include <boost/asio.hpp>
#include <condition_variable>
#include <mutex>
#include <string>
#include <unordered_map>

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
	std::istream &in;
	std::ostream &out;

	uint port;
	std::string server_name;

	uint cid;

	uint retransmit_limit;

	/** Connection */

	bool is_connected() const;
	void set_connected(bool connected);
	void touch_connection();

	static const uint CONNECTION_EXPIRY_TIME_SEC = 1;
	static const uint CONNECTION_RETRY_TIME_SEC  = 1;
	mutable std::mutex mutex_connected;
	bool connected;

	boost::asio::io_service io_service;

	/** TCP */

	bool connect_tcp();
	bool read_init_message();

	boost::asio::ip::tcp::socket tcp_socket;
	boost::asio::ip::tcp::resolver tcp_resolver;

	/** UDP */

	void connect_udp();
	void keep_alive_routine();

	static const uint KEEP_ALIVE_TIMEOUT_MS = 500;

	void server_interaction_routine();
	void insert_input();
	void manage_messages();
	void print_data();

	std::unordered_map<uint, std::string> messages;
	std::string input_buffer;
	std::string output_buffer;

	uint   acknowledged;
	uint   sent;
	uint   expected;
	size_t window_size;

	bool ask_retransmit(uint number);
	bool send_data(const std::string &data, uint number);

	static const size_t MIN_DATA_SIZE = 16;

	boost::asio::ip::udp::socket   udp_socket;
	boost::asio::ip::udp::resolver udp_resolver;
	boost::asio::ip::udp::endpoint udp_endpoint;

	static const size_t BUFFER_SIZE = 65536 << 2;
	static const size_t MSG_SIZE    = 65536;
};

#endif // EMCLIENT_H

