#include "System/AbstractServer.h"

AbstractServer::AbstractServer() :
	current_cid(DEFAULT_FIRST_CID)
{}

AbstractServer::~AbstractServer()
{}

uint AbstractServer::get_next_cid()
{
	return current_cid++;
}