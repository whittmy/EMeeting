#include <boost/lexical_cast.hpp>
#include <cstdio>
#include <iostream>
#include <thread>
#include <unistd.h>

#include "Client/EMClient.h"
#include "System/AbstractServer.h"
#include "System/DataBuffer.h"
#include "System/Messages.h"
#include "System/Utils.h"

/**
 * \class EMClient
 */

EMClient::EMClient(std::istream &in, std::ostream &out) :
	in(in),
	out(out),
	port(EM::Default::PORT),
	retransmit_limit(EM::Default::RETRANSMIT_LIMIT),

	io_service(),
	tcp_socket(io_service),
	tcp_resolver(io_service),

	udp_socket(io_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 0)),
	udp_resolver(io_service),

	datagram_client_server_number(1),
	datagram_client_sent_number(0),
	datagram_client_input_number(0),

	datagram_server_client_number(0)
{}

EMClient::~EMClient()
{}

void EMClient::set_port(uint port)
{
	this->port = port;
}

uint EMClient::get_port() const
{
	return port;
}

void EMClient::set_server_name(const std::string server_name)
{
	this->server_name = server_name;
}

std::string EMClient::get_server_name() const
{
	return server_name;
}

void EMClient::set_retransmit_limit(uint retransmit_limit)
{
	this->retransmit_limit = retransmit_limit;
}

uint EMClient::get_retransmit_limit() const
{
	return retransmit_limit;
}

void EMClient::start()
{
	io_service.run();

	std::cerr << "Connecting with " << get_server_name() << " on port " << get_port() << "\n";

	boost::system::error_code error;
	boost::array<char, EM::Messages::LENGTH> buf;

	bool connected = false;

	std::string received_message;

	boost::asio::deadline_timer timer(io_service);

	while (true) {
		repeat_twice {
			connected = connect_tcp();

			if (connected)
				std::thread (&EMClient::connect_udp, this).detach();
			else
				continue;

			static const uint MAX_DELAY = 3;
			uint delay = 0;

			while (connected) {
				size_t bytes_read =
					tcp_socket.read_some(boost::asio::buffer(buf), error);

				if (error) {
					/** When error is something other than nothing to read */
					if (error.value() != boost::asio::error::eof ||
					    delay > MAX_DELAY)
						connected = false;
					else
						++delay;
				} else {
					delay = 0;
					received_message =
						std::string(buf.begin(),
							buf.begin() + bytes_read);
// 					std::cerr << received_message;
				}
				timer.expires_from_now(
					boost::posix_time::milliseconds(
						AbstractServer::SEND_INFO_TIMEOUT_MS));
				timer.wait();
			}
			std::cerr << "Disconnected!\n";
		}
		sleep(1);
	}
}

void EMClient::quit()
{}

bool EMClient::connect_tcp()
{
	std::cerr << "Establishing connection... ";

	boost::system::error_code error;

	/** If this throws an exception, your computer just exploded */
	boost::asio::ip::tcp::resolver::query query(
		get_server_name(), boost::lexical_cast<std::string>(get_port()));

	boost::asio::ip::tcp::resolver::iterator endpoint_iterator =
		tcp_resolver.resolve(query);
	boost::asio::connect(tcp_socket, endpoint_iterator, error);

	if (error)
		std::cerr << "unable to connect.\n";
	else
		std::cerr << "connected!\n";

	return !error && read_init_message();
}

bool EMClient::read_init_message()
{
	boost::array<char, EM::Messages::LENGTH> buf;
	boost::system::error_code error;

	std::cerr << "Reading network initialization message... ";

	size_t length = tcp_socket.read_some(boost::asio::buffer(buf), error);
	std::memset((void *) (&buf[0] + length), (int) '\0', EM::Messages::LENGTH - length);

	if (error)
		return false;

	std::string received_message(buf.begin(), buf.begin() + length);

	if (!(length > 6 && received_message.substr(0, 6) ==
		EM::Messages::Client.substr(0, 6)))
		return false;

	std::cerr << "received message: " << received_message;

	received_message =
		EM::trimmed(EM::without_endline(received_message.substr(6)));

	try {
		cid = boost::lexical_cast<uint>(received_message);
	} catch (boost::bad_lexical_cast) {
		std::cerr << "Received invalid client id: \""
				<< received_message << "\"\n";
		return false;
	}

	return true;
}

void EMClient::connect_udp()
{
	std::cerr << "Establishing UDP connection...\n";

	char request[EM::Messages::LENGTH];
	std::sprintf(request, EM::Messages::Client.c_str(), cid);

	boost::system::error_code error;

	do {
		udp_endpoint = *udp_resolver.resolve({
			boost::asio::ip::udp::v4(),
			get_server_name(),
			boost::lexical_cast<std::string>(get_port())});

		for (int i = 0; i == 0 || (i == 1 && error); ++i)
			udp_socket.send_to(boost::asio::buffer(request, std::strlen(request)),
				udp_endpoint, boost::asio::ip::udp::socket::message_flags(0),
				error);
		if (error)
			sleep(1);
	} while (error);

	std::cerr << "UDP connected!\n";

	std::thread (&EMClient::send_data_to_server, this).detach();
	std::thread (&EMClient::read_data_from_server, this).detach();

	return keep_alive_routine();
}

void EMClient::send_data_to_server()
{
	DataBuffer buffer(BUFFER_SIZE);

	char input[MSG_SIZE];

	/** Small, initial value */
	window_size = 64;

	while (in.getline(input, MSG_SIZE)) {
		buffer.insert({input, std::strlen(input)});
		EM::Data data = buffer.get_data(datagram_client_server_number, window_size);
		++datagram_client_input_number;

		do {
			do {
				if (!send_datagram(data.data, data.length,
					datagram_client_server_number)) {
					buffer.clear();
					return;
				} else {
					++datagram_client_sent_number;
				}

				if (!wait_for_ack()) {
					buffer.clear();
					return;
				}
			} while (datagram_client_server_number <= datagram_client_sent_number);
		} while (window_size < buffer.get_size());
	}

	std::cerr << "Sending data completed (end-of-file).\n";
}

void EMClient::read_data_from_server()
{
	char *buffer = new char[BUFFER_SIZE];

	uint ack = 0, nr = 0;
	size_t win = 0;
	size_t index;
	size_t length;
	boost::system::error_code error;

	while (true) {
		length =
			udp_socket.receive_from(boost::asio::buffer(buffer, BUFFER_SIZE),
			udp_endpoint, boost::asio::ip::udp::socket::message_flags(0), error);

		std::cerr << "READ " << buffer;

		if (error) {
			delete[] buffer;
			return;
		}

		EM::Messages::Type type = EM::Messages::get_type(buffer, length);

		data_mutex.lock();
		switch (type) {
			case EM::Messages::Type::Data:
				if (!EM::Messages::read_data(
					buffer, EM::Messages::LENGTH, nr, ack, win))
					break;
				datagram_client_server_number = ack;
				window_size = win;
				if (nr > datagram_server_client_number &&
					datagram_server_client_number != 0) {
					ask_retransmit(datagram_server_client_number);
				} else {
					data_mutex.unlock();
					for (index = 0; index < std::strlen(buffer) &&
						buffer[index] != '\n'; ++index) {}
					out << buffer + index + 1;
					data_mutex.lock();
				}
				break;
			case EM::Messages::Type::Ack:
				if (!EM::Messages::read_ack(buffer, length, ack, win))
					break;
				datagram_client_server_number = ack;
				window_size = win;
				break;
			default:;
		}
		data_mutex.unlock();
	}
}

void EMClient::keep_alive_routine()
{
	boost::asio::deadline_timer timer(
		io_service, boost::posix_time::milliseconds(KEEP_ALIVE_FREQUENCY));

	char request[EM::Messages::LENGTH];
	std::sprintf(request, EM::Messages::KeepAlive.c_str());

	boost::system::error_code error;

	while (true) {
		timer.expires_from_now(boost::posix_time::milliseconds(KEEP_ALIVE_FREQUENCY));
		timer.wait();

// 		std::cerr << "Sending KEEPALIVE\n";
		udp_socket.send_to(
			boost::asio::buffer(request, std::strlen(request)), udp_endpoint,
			boost::asio::ip::udp::socket::message_flags(0), error);
		if (error)
			return connect_udp();
	}
}

bool EMClient::ask_retransmit(uint number)
{
	char message[EM::Messages::LENGTH];
	std::sprintf(message, EM::Messages::Retransmit.c_str(), number);

	boost::system::error_code error;

	std::cerr << "SEND " << message;

	udp_socket.send_to(
		boost::asio::buffer(message, std::strlen(message)), udp_endpoint,
		boost::asio::ip::udp::socket::message_flags(0), error);

	return (bool) !error;
}

bool EMClient::send_datagram(void *data, size_t length, uint number)
{
	boost::array<char, MSG_SIZE> output;
	boost::system::error_code error;

	std::sprintf(output.c_array() , EM::Messages::Upload.c_str(), number);
	size_t message_length = std::strlen(output.c_array());

	std::cerr << "SEND " << output.c_array();

	std::memcpy(output.c_array() + message_length, data, length);

	uint bytes_sent =
		udp_socket.send_to(
			boost::asio::buffer(output, message_length + length),
			udp_endpoint,
			boost::asio::ip::udp::socket::message_flags(0), error);

	if (error || bytes_sent < message_length + length) {
		std::cerr << "Unable to send data to server.\n";
		return false;
	}

	return true;
}

bool EMClient::wait_for_ack()
{
	data_mutex.lock();
	while (datagram_client_server_number <= datagram_client_sent_number) {
		data_mutex.unlock();
		data_mutex.lock();
	}
	data_mutex.unlock();
	return true;
}
