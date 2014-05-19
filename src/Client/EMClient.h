#ifndef EMCLIENT_H
#define EMCLIENT_H

#include <string>

class EMClient
{
public:
	EMClient();

	void set_port(uint port);
	void set_server_name(const std::string server_name);

	void set_retransmit_limit(uint retransmit_limit);

private:
	uint port;
	std::string server_name;

	uint retransmit_limit;
};

#endif // EMCLIENT_H
