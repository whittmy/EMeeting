#include "EMClient.h"

namespace EM {
	namespace Default {
		namespace Client {
			const uint PORT = 0;

			const uint RETRANSMIT_LIMIT = 10;
		}
	}
}

EMClient::EMClient() :
	port(EM::Default::Client::PORT),
	retransmit_limit(EM::Default::Client::RETRANSMIT_LIMIT)
{}

void EMClient::set_port(uint port)
{
	this->port = port;
}

void EMClient::set_server_name(const std::string server_name)
{
	this->server_name = server_name;
}

void EMClient::set_retransmit_limit(uint retransmit_limit)
{
	this->retransmit_limit = retransmit_limit;
}


