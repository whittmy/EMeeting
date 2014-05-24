#ifndef ARGSMANAGER_H
#define ARGSMANAGER_H

#include <set>
#include <vector>

#include "System/Utils.h"

namespace EM {
	enum class Arg : uint8_t {
		Help,

		Port,
		ServerName,

		FifoSize,
		FifoLowWatermark,
		FifoHighWatermark,

		BufferLength,
		RetransmitLimit,

		TxInterval,

		Undefined,
	};

	namespace Args {
		Arg from_string(const std::string &cmd);
	}
}

class ArgsManager {
public:
	ArgsManager(int argc, char **argv);

	bool finished() const;
	uint get_uint();
	std::string get_string();
	EM::Arg get_arg();
	bool arg_set(EM::Arg arg) const;
	std::string get_previous_arg() const;

private:
	void set_previous_arg(const std::string &previous_arg);

	std::vector<std::string> args;
	std::set<EM::Arg> args_changed;

	size_t current_index;
	std::string previous_arg;
	std::string waiting_string;
};

#endif // ARGSMANAGER_H
