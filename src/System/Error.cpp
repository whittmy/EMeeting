#include <map>

#include "System/Error.h"
#include "System/Strings.h"

static const std::map<EM::Error, std::string> error_to_string {
	{EM::Error::ConflictingArgs, EM::Strings::Errors::ConflictingArgs},
	{EM::Error::InvalidArg,      EM::Strings::Errors::InvalidArg},
	{EM::Error::UnknownArg,      EM::Strings::Errors::UnknownArg},
	{EM::Error::ArgLacking,      EM::Strings::Errors::ArgLacking},
	{EM::Error::NoServerName,    EM::Strings::Errors::NoServerName},
};

std::string EM::Errors::to_string(EM::Error e)
{
	return error_to_string.at(e);
}
