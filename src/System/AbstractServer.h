#ifndef ABSTRACT_SERVER_H
#define ABSTRACT_SERVER_H

#include <string>
#include <sys/types.h>

class Connection
{
public:
	virtual ~Connection();

	virtual std::string get_name() const = 0;
};

class AbstractServer
{
public:
	AbstractServer();
	virtual ~AbstractServer();

	virtual uint get_next_cid();
	virtual void add_client(uint cid) = 0;
	virtual void on_connection_established(uint cid, Connection *connection) = 0;
	virtual void on_connection_lost(uint cid) = 0;

	static const uint SEND_INFO_FREQUENCY = 1;

private:
	static const int DEFAULT_FIRST_CID = 1;

	uint current_cid;
};

#endif // ABSTRACT_SERVER_H