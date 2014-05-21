#include <csignal>
#include <iostream>

#include "System/Error.h"
#include "System/SignalHandler.h"

std::map<int, std::function<void(void)> > SignalHandler::exit_functions;

void SignalHandler::setup(int signal_number, std::function<void(void)> exit_function)
{
	if (std::signal((int) SIGINT, handle) == SIG_ERR) {
		std::cerr << EM::Errors::to_string(EM::Error::SignalSettingFailed) << "\n";
		exit(EXIT_SUCCESS);
	}

	exit_functions[signal_number] = exit_function;
}

void SignalHandler::handle(int signal_number)
{
	exit_functions[signal_number]();
}

