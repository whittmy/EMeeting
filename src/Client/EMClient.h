#ifndef EMCLIENT_H
#define EMCLIENT_H

#include <string>
#include <iostream>

class EMClient
{
public:
	EMClient(std::istream &in, std::ostream &out);

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

	uint retransmit_limit;
};

#endif // EMCLIENT_H
