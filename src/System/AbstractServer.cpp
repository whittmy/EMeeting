#include <iostream>

#include "System/AbstractServer.h"

/**
 * \class Connection
 */

Connection::~Connection()
{}


/**
 * \class AbstractServer
 */

AbstractServer::AbstractServer() :
	current_cid(DEFAULT_FIRST_CID)
{}

AbstractServer::~AbstractServer()
{}

uint AbstractServer::get_next_cid()
{
	return current_cid++;
}