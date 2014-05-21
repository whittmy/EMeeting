#include <iostream>

#include "Server/EMServer.h"
#include "System/ArgsManager.h"
#include "System/Error.h"
#include "System/Strings.h"
#include "System/Utils.h"

int main(int argc, char **argv)
{
	EMServer em_server;
	ArgsManager args_manager(argc - 1, argv + 1);

	while (!args_manager.finished()) {
		switch (args_manager.get_arg()) {
			case EM::Arg::Help:
				std::cout << EM::Strings::Server::HelpMessage;
				return EXIT_SUCCESS;

			case EM::Arg::Port:
				em_server.set_port(args_manager.get_uint());
				break;
			case EM::Arg::FifoSize:
				em_server.set_fifo_size(args_manager.get_uint());
				break;
			case EM::Arg::FifoLowWatermark:
				em_server.set_fifo_low_watermark(args_manager.get_uint());
				break;
			case EM::Arg::FifoHighWatermark:
				em_server.set_fifo_high_watermark(args_manager.get_uint());
				break;

			case EM::Arg::BufferLength:
				em_server.set_buffer_length(args_manager.get_uint());
				break;

			default:
				std::cerr << EM::Errors::to_string(EM::Error::UnknownArg) << ": "
				          << args_manager.get_previous_arg() << "\n";
				return EXIT_SUCCESS;
		}
	}

	em_server.start();

	return EXIT_SUCCESS;
}