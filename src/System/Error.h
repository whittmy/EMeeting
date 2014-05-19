#ifndef ERROR_H
#define ERROR_H

#include <string>

namespace EM {
	enum class Error : uint8_t {
		ConflictingArgs,
		InvalidArg,
		UnknownArg,
		ArgLacking,
		NoServerName,
	};

	namespace Errors {
		std::string to_string(Error e);
	}
}

#endif // ERROR_H
