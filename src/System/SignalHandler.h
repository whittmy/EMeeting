#ifndef SIGNALHANDLER_H
#define SIGNALHANDLER_H

#include <csignal>
#include <functional>
#include <map>

class SignalHandler
{
public:
	static void setup(int signal_number, std::function<void(void)> exit_function);
	static void handle(int signal_number);

private:
	SignalHandler() = delete;
	SignalHandler(const SignalHandler &) = delete;

	static std::map<int, std::function<void(void)> > exit_functions;
};

#endif // SIGNALHANDLER_H
