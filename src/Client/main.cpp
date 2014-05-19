#include <iostream>

#include "Client/EMClient.h"
#include "System/ArgsManager.h"
#include "System/Error.h"
#include "System/Strings.h"

int main(int argc, char **argv)
{
	EMClient em_client;
	ArgsManager args_manager(argc - 1, argv + 1);

	while (!args_manager.finished()) {
		switch (args_manager.get_arg()) {
			case EM::Arg::Help:
				std::cout << EM::Strings::Client::HelpMessage;
				return EXIT_SUCCESS;

			case EM::Arg::Port:
				em_client.set_port(args_manager.get_uint());
				break;
			case EM::Arg::ServerName:
				em_client.set_server_name(args_manager.get_string());
				break;

			case EM::Arg::BufferLength:
				em_client.set_retransmit_limit(args_manager.get_uint());
				break;

			default:
				std::cerr << EM::Errors::to_string(EM::Error::UnknownArg) << ": "
				          << args_manager.get_previous_arg() << "\n";
				return EXIT_SUCCESS;
		}
	}

	if (!args_manager.arg_set(EM::Arg::ServerName)) {
		std::cerr << EM::Errors::to_string(EM::Error::NoServerName) << "\n";
		return EXIT_SUCCESS;
	}

	return EXIT_SUCCESS;
}