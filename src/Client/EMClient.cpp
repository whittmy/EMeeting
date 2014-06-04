#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <cstdio>
#include <iostream>
#include <thread>
#include <unistd.h>

#include "Client/EMClient.h"
#include "System/AbstractServer.h"
#include "System/Logging.h"
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
	udp_resolver(io_service)
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

	info() << "Connecting with " << get_server_name() << " on port " << get_port() << "\n";

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

			info() << "Connected!\n";

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
					info() << received_message;
				}
				timer.expires_from_now(
					boost::posix_time::milliseconds(
						AbstractServer::SEND_INFO_TIMEOUT_MS));
				timer.wait();
			}
			info() << "Disconnected!\n";
		}
		timer.expires_from_now(
			boost::posix_time::seconds(CONNECTION_RETRY_TIME_SEC));
		timer.wait();
	}
}

void EMClient::quit()
{}

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

bool EMClient::connect_tcp()
{
	log() << "Establishing connection... ";

	boost::system::error_code error;

	/** If this throws an exception, your computer just exploded */
	boost::asio::ip::tcp::resolver::query query(
		get_server_name(), boost::lexical_cast<std::string>(get_port()));

	boost::asio::ip::tcp::resolver::iterator endpoint_iterator =
		tcp_resolver.resolve(query);
	boost::asio::connect(tcp_socket, endpoint_iterator, error);

	if (error)
		log() << "unable to connect.\n";

	return !error && read_init_message();
}

bool EMClient::read_init_message()
{
	boost::array<char, EM::Messages::LENGTH> buf;
	boost::system::error_code error;

	log() << "Reading network initialization message... ";

	size_t length = tcp_socket.read_some(boost::asio::buffer(buf), error);

	if (error)
		return false;

	std::string received_message(buf.begin(), buf.begin() + length);

	return EM::Messages::read_client(received_message, cid);
}

void EMClient::connect_udp()
{
	log() << "Establishing UDP connection...\n";

	std::string request(EM::Messages::LENGTH, '\0');
	std::sprintf(&request[0], EM::Messages::Client.c_str(), cid);

	boost::system::error_code error;

	boost::asio::deadline_timer timer(io_service);

	do {
		udp_endpoint = *udp_resolver.resolve({
			boost::asio::ip::udp::v4(),
			get_server_name(),
			boost::lexical_cast<std::string>(get_port())});

		for (int i = 0; i == 0 || (i == 1 && error); ++i)
			udp_socket.send_to(boost::asio::buffer(request),
				udp_endpoint, boost::asio::ip::udp::socket::message_flags(0),
				error);
		if (error) {
			timer.expires_from_now(
				boost::posix_time::seconds(CONNECTION_RETRY_TIME_SEC));
			timer.wait();
		}
	} while (error);

	log() << "UDP connected!\n";

	touch_connection();

	std::thread (&EMClient::server_interaction_routine, this).detach();

	return keep_alive_routine();
}

void EMClient::keep_alive_routine()
{
	boost::asio::deadline_timer timer(io_service);

	char request[EM::Messages::LENGTH];
	std::sprintf(request, EM::Messages::KeepAlive.c_str());

	boost::system::error_code error;

	while (true) {
		timer.expires_from_now(boost::posix_time::milliseconds(KEEP_ALIVE_TIMEOUT_MS));
		timer.wait();

		udp_socket.send_to(
			boost::asio::buffer(request, std::strlen(request)), udp_endpoint,
			boost::asio::ip::udp::socket::message_flags(0), error);
		if (error)
			return connect_udp();
	}
}

void EMClient::server_interaction_routine()
{
	messages.clear();
	input_buffer.clear();

	acknowledged = 0;
	sent         = 0;
	expected     = 0;
	window_size  = 64;

	boost::system::error_code error;
	boost::array<char, BUFFER_SIZE> buf;

	/** We send anything to get going */
// 	insert_input();
// 	manage_messages();

	while (is_connected()) {
		insert_input();

		size_t length =
			udp_socket.receive_from(boost::asio::buffer(buf),
			udp_endpoint, boost::asio::ip::udp::socket::message_flags(0), error);
		output_buffer = std::string(buf.begin(), buf.begin() + length);

		if (error)
			set_connected(false);

		switch (EM::Messages::get_type(output_buffer)) {
			case EM::Messages::Type::Ack: {
				uint ack;
				size_t win;
				if (!EM::Messages::read_ack(output_buffer, ack, win))
					break;
				log() << "READ " << output_buffer;

				acknowledged = ack;
				window_size  = win;

				manage_messages();

				break;
			}
			case EM::Messages::Type::Data: {
				uint nr, ack;
				size_t win;
				if (!EM::Messages::read_data(output_buffer, nr, ack, win))
					break;

				acknowledged = std::max(ack, acknowledged);
				window_size  = win;

				size_t index =
					output_buffer.find("\n");
				if (index >= output_buffer.size()) {
					info() << "READ invalid DATA\n";
					break;
				}
				out << output_buffer.substr(index + 1);
				log() << "READ " << output_buffer.substr(0, index + 1);

				if (nr > expected && nr - expected <= get_retransmit_limit()) {
					ask_retransmit(expected);
				} else {
					expected = nr + 1;
					manage_messages();
				}

				break;
			}
			default:;
				/** Ignored */

		}
	}
}

void EMClient::insert_input()
{
	std::string input;
	while (input_buffer.size() < BUFFER_SIZE
		&& std::getline(in, input))
		input_buffer += input + "\n";
}

void EMClient::manage_messages()
{
	if (acknowledged < sent) {
		log() << "Retransmitting\n";
		for (uint i = acknowledged; i < sent; ++i)
			if (messages.find(i) != messages.end())
				send_data(messages[i], i);
		/** We don't want too many retransmits */
		debug() << "acknowledged: " << acknowledged << "\n";
		++acknowledged;
		debug() << "and acknowledged: " << acknowledged << "\n";
	} else if (window_size >= MIN_DATA_SIZE && input_buffer.size() >= MIN_DATA_SIZE) {
		size_t length = std::min(input_buffer.size(), window_size);
		while (length % sizeof(EM::data_t) != 0)
			--length;
		messages[sent] = input_buffer.substr(0, length);
		input_buffer   = input_buffer.substr(length);
		send_data(messages[sent], sent);
		++sent;
		window_size -= length;
	}

	auto old_msg_it = messages.find(sent - get_retransmit_limit() - 1);
	if (old_msg_it != messages.end())
		messages.erase(old_msg_it);
}

bool EMClient::ask_retransmit(uint number)
{
	char message[EM::Messages::LENGTH];
	std::sprintf(message, EM::Messages::Retransmit.c_str(), number);

	boost::system::error_code error;

	log() << "SEND " << message;

	udp_socket.send_to(
		boost::asio::buffer(message, std::strlen(message)), udp_endpoint,
		boost::asio::ip::udp::socket::message_flags(0), error);

	return (bool) !error;
}

bool EMClient::send_data(const std::string &data, uint number)
{
	std::string output(EM::Messages::LENGTH, '\0');
	boost::system::error_code error;

	std::sprintf(&output[0] , EM::Messages::Upload.c_str(), number);
	output = output.substr(0, output.find("\n") + 1);

	log() << "SEND (" << data.size() << ") " << output;

	output += data;

	uint bytes_sent =
		udp_socket.send_to(
			boost::asio::buffer(output),
			udp_endpoint,
			boost::asio::ip::udp::socket::message_flags(0), error);

	if (error || bytes_sent < output.size()) {
		warn() << "Unable to send data to server.\n";
		return false;
	}

	return true;
}
