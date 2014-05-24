#ifndef ABSTRACT_SERVER_H
#define ABSTRACT_SERVER_H

#include <sys/types.h>

class AbstractServer
{
public:
	AbstractServer();
	virtual ~AbstractServer();

	uint get_next_cid();
	virtual void add_client(uint cid) = 0;
	virtual void on_connection_established(uint cid) = 0;
	virtual void on_connection_lost(uint cid) = 0;

private:
	static const int DEFAULT_FIRST_CID = 1;

	uint current_cid;
};

#endif // ABSTRACT_SERVER_H