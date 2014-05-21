#include <iostream>
#include <unordered_map>

#include "System/ArgsManager.h"
#include "System/Error.h"
#include "System/Strings.h"

/**
 * \namespace EM::Args
 */

static const std::unordered_map<std::string, EM::Arg> string_to_arg {
	{EM::Strings::Args::Help,              EM::Arg::Help},
	{EM::Strings::Args::Port,              EM::Arg::Port},
	{EM::Strings::Args::ServerName,        EM::Arg::ServerName},
	{EM::Strings::Args::FifoSize,          EM::Arg::FifoSize},
	{EM::Strings::Args::FifoLowWatermark,  EM::Arg::FifoLowWatermark},
	{EM::Strings::Args::FifoHighWatermark, EM::Arg::FifoHighWatermark},
	{EM::Strings::Args::BufferLength,      EM::Arg::BufferLength},
	{EM::Strings::Args::TxInterval,        EM::Arg::TxInterval},
};

EM::Arg EM::Args::from_string(const std::string &cmd)
{
	if (string_to_arg.find(cmd) == string_to_arg.end())
		return EM::Arg::Undefined;
	else
		return string_to_arg.at(cmd);
}

/**
 * \class ArgsManager
 */

ArgsManager::ArgsManager(int argc, char **argv)
{
	for (int i = 0; i < argc; ++i) {
		std::string str = std::string(argv[i]);
		if (str.length() > 0)
			args.push_back(str);
	}

	current_index = 0;
}

bool ArgsManager::finished() const
{
	return current_index == args.size();
}

uint ArgsManager::get_uint()
{
	if (finished()) {
		std::cerr << EM::Strings::Error << EM::Errors::to_string(EM::Error::ArgLacking)
		          << ":" << get_previous_arg() << "\n";
		exit(EXIT_SUCCESS);
	} else {
		uint current_arg = std::stoi(args[current_index++]);
		return current_arg;
	}
}

std::string ArgsManager::get_string()
{
	if (finished()) {
		std::cerr << EM::Strings::Error << EM::Errors::to_string(EM::Error::ArgLacking)
		          << ":" << get_previous_arg() << "\n";
		exit(EXIT_SUCCESS);
	} else {
		std::string current_arg = args[current_index++];
		return current_arg;
	}
}

EM::Arg ArgsManager::get_arg()
{
	std::string current_arg = get_string();

	EM::Arg arg = EM::Args::from_string(current_arg);

	if (arg_set(arg)) {
		std::cerr << EM::Strings::Error
		          << EM::Errors::to_string(EM::Error::ConflictingArgs)
		          << ":" << current_arg << "\n";
	}

	args_changed.insert(arg);
	set_previous_arg(current_arg);

	return arg;
}

bool ArgsManager::arg_set(EM::Arg arg) const
{
	return args_changed.find(arg) != args_changed.end();
}

std::string ArgsManager::get_previous_arg() const
{
	return previous_arg;
}

void ArgsManager::set_previous_arg(const std::string &previous_arg)
{
	this->previous_arg = previous_arg;
}
