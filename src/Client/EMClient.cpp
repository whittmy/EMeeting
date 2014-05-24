#include <boost/lexical_cast.hpp>
#include <cstdio>
#include <unistd.h>

#include "Client/EMClient.h"
#include "System/AbstractServer.h"
#include "System/Messages.h"
#include "System/Utils.h"

EMClient::EMClient(std::istream &in, std::ostream &out) :
	in(in),
	out(out),
	port(EM::Default::PORT),
	retransmit_limit(EM::Default::RETRANSMIT_LIMIT),

	io_service(),
	tcp_resolver(io_service)
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

	std::cerr << "connecting with " << get_server_name() << " on port " << get_port() << "\n";

	/** If this throws an exception, your computer just exploded */
	boost::asio::ip::tcp::resolver::query query(
		get_server_name(), boost::lexical_cast<std::string>(get_port()));

	static const size_t BUFFER_SIZE = 128;

	boost::system::error_code error;
	boost::array<char, BUFFER_SIZE> buf;

	while (true) {
		repeat_twice {
			std::cerr << "establishing connection...\n";

			boost::asio::ip::tcp::resolver::iterator endpoint_iterator =
				tcp_resolver.resolve(query);

			boost::asio::ip::tcp::socket socket(io_service);
			boost::asio::connect(socket, endpoint_iterator, error);

			if (!error) {
				std::cerr << "connection established\n";
			} else {
				std::cerr << "unable to connect\n";
				continue;
			}

			bool connected = true;

			std::cerr << "reading network initialization message...\n";

			size_t length = socket.read_some(boost::asio::buffer(buf), error);
			std::memset(buf.c_array() + length, (int) '\0', BUFFER_SIZE - length);

			if (error)
				connected = false;

			std::string received_message(buf.begin(), buf.begin() + length);

			connected &= length > 6 && received_message.substr(0, 6) ==
			             EM::Messages::Client.substr(0, 6);

			std::cerr << "received message: " << received_message;

			received_message =
				EM::trimmed(EM::without_endline(received_message.substr(6)));

			try {
				cid = boost::lexical_cast<uint>(received_message);
			} catch (boost::bad_lexical_cast) {
				std::cerr << "received invalid client id: \""
				          << received_message << "\"\n";
				connected = false;
			}

			if (connected)
				connect_udp();

			while (connected) {
				socket.read_some(boost::asio::buffer(buf), error);

				if (error) {
					/** When error is something other than nothing to read */
					if (error.value() != 2) {
						connected = false;
						std::cerr << error.message()
						          << " (" << error.value()
						          << ")\n";
					}
				} else {
					received_message = std::string(buf.begin(), buf.end());
					out << received_message;
				}
				sleep(AbstractServer::SEND_INFO_FREQUENCY);
			}
			std::cerr << "disconnected!\n";
		}
		sleep(10);
	}
}

void EMClient::quit()
{}

void EMClient::connect_udp()
{}