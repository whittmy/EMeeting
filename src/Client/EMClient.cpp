#include <boost/bind.hpp>
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

	acknowledged(0),

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

	std::string received_message;

	boost::asio::deadline_timer timer(io_service);

	while (true) {
		repeat_twice {
			set_connected(connect_tcp());

			if (is_connected())
				std::thread (&EMClient::connect_udp, this).detach();
			else
				continue;

			static const uint MAX_DELAY = 3;
			uint delay = 0;

			while (is_connected()) {
				size_t bytes_read =
					tcp_socket.read_some(boost::asio::buffer(buf), error);

				if (error) {
					/** When error is something other than nothing to read */
					if (error.value() != boost::asio::error::eof ||
						delay > MAX_DELAY)
						set_connected(false);
					else
						++delay;
				} else {
					delay = 0;
					received_message =
						std::string(buf.begin(),
							buf.begin() + bytes_read);
					std::cerr << received_message;
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

	touch_connection();

	std::thread (&EMClient::server_interaction, this).detach();

	return keep_alive_routine();
}

void EMClient::server_interaction()
{
	std::string input_buffer;
	boost::asio::buffer<char, MSG_SIZE> server_buffer;
	boost::system::error_code error;

	size_t length;

	/** Small, initial window size */
	window_size = 64;

	while (is_connected()) {
		insert_input(input_buffer);

		length =
			udp_socket.receive_from(boost::asio::buffer(server_buffer, BUFFER_SIZE),
			udp_endpoint, boost::asio::ip::udp::socket::message_flags(0), error);

		if (error)
			set_connected(false);

		switch (EM::Messages::get_type(server_buffer.c_array(), server_buffer.size())) {
			case EM::Messages::Type::Ack: {
				uint ack;
				size_t win;
				if (!EM::Messages::read_ack(server_buffer, length, ack, win))
					break;
				std::cerr << "READ "
					<< std::string(server_buffer.c_array(),
						server_buffer.size());

				acknowledged = ack;
				window_size  = win;
			}
			case EM::Messages::Type::Data: {
				uint nr, ack;
				size_t win;
				if (!EM::Messages::read_data(server_buffer, length, nr, ack, win))
					break;

				for (index = 0; index < length &&
					server_buffer[index] != '\n'; ++index) {}
				++index;
				out << server_buffer.substr(index);

				std::cerr << "READ ";
			}

		} // nr ack win
	}
}

void EMClient::insert_input(std::string &str, size_t length)
{
	std::string input;
	while (str.size() < length && in.getline(input))
		str += input;
}

void EMClient::send_data_to_server()
{
	DataBuffer buffer(BUFFER_SIZE);

	char input[MSG_SIZE];

	/** Small, initial value */
	window_size = 64;

	while (true) {
		while (buffer.get_size() < MSG_SIZE && in.getline(input, MSG_SIZE))
			buffer.insert({input, std::strlen(input)});

		EM::Data data = buffer.get_data(datagram_client_server_number, window_size);

		if (data.length == 0)
			continue;

		++datagram_client_input_number;

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
		if (window_size == 0) {
			if (!wait_for_window()) {
				buffer.clear();
				return;
			}
		}
	}
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

		touch_connection();

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
				std::cerr << "READ DATA " << ack << " " << win << " (length: "
					<< length << ")\n";

				if (nr > datagram_server_client_number &&
					datagram_server_client_number != 0) {
					ask_retransmit(datagram_server_client_number);
				} else {
					data_mutex.unlock();

					data_mutex.lock();
				}
				break;
			case EM::Messages::Type::Ack:
				std::cerr << "READ " << buffer;
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

bool EMClient::is_connected() const
{
	mutex_connected.lock();
	bool result = connected;
	mutex_connected.unlock();
	return result;
}

void EMClient::set_connected(bool connected)
{
	mutex_connected.lock();
	this->connected = connected;
	mutex_connected.unlock();
}

void EMClient::touch_connection()
{
	static boost::asio::deadline_timer timer(io_service);
	timer.expires_from_now(boost::posix_time::seconds(CONNECTION_EXPIRY_TIME_SEC));
	timer.async_wait(boost::bind(&EMClient::set_connected, this, false));
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

bool EMClient::send_datagram(const std::string &data, uint number)
{
	std::string output(EM::Messages::LENGTH, " ");
	boost::system::error_code error;

	std::sprintf(&output[0] , EM::Messages::Upload.c_str(), number);
	output.resize(std::strlen(output));

	std::cerr << "SEND " << output;

	output += data;

	uint bytes_sent =
		udp_socket.send_to(
			boost::asio::buffer(output),
			udp_endpoint,
			boost::asio::ip::udp::socket::message_flags(0), error);

	if (error || bytes_sent < output.size()) {
		std::cerr << "Unable to send data to server.\n";
		return false;
	}

	return true;
}
