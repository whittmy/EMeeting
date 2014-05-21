#include "Client/EMClient.h"
#include "System/Messages.h"
#include "System/Utils.h"

EMClient::EMClient(std::istream &in, std::ostream &out) :
	in(in),
	out(out),
	port(EM::Default::PORT),
	retransmit_limit(EM::Default::RETRANSMIT_LIMIT)
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

void EMClient::quit()
{
}