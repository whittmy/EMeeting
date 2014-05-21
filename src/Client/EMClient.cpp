#include "EMClient.h"

namespace EM {
	namespace Default {
		namespace Client {
			const uint PORT = 0;

			const uint RETRANSMIT_LIMIT = 10;
		}
	}
}

EMClient::EMClient(std::istream &in, std::ostream &out) :
	in(in),
	out(out),
	port(EM::Default::Client::PORT),
	retransmit_limit(EM::Default::Client::RETRANSMIT_LIMIT)
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

}