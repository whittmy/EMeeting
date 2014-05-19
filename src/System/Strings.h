#ifndef STRINGS_H
#define STRINGS_H

#include <string>

namespace EM {
	namespace Strings {
		namespace Args {
			const std::string Help              = "-h";
			const std::string Port              = "-p";
			const std::string ServerName        = "-s";
			const std::string FifoSize          = "-F";
			const std::string FifoLowWatermark  = "-L";
			const std::string FifoHighWatermark = "-H";
			const std::string BufferLength      = "-X";
			const std::string TxInterval        = "-i";
		}

		const std::string Error = "Error:";

		namespace Errors {
			const std::string ConflictingArgs = "Conflicting arguments";
			const std::string InvalidArg      = "Invalid argument";
			const std::string UnknownArg      = "Unknown argument";
			const std::string ArgLacking      = "Lacking an argument";
			const std::string NoServerName    =
				std::string("No server name given (use ") + Args::ServerName
				+ std::string(")");
		}

		namespace Server {
			const std::string HelpMessage =
				std::string("Usage: ./server [OPTION]...\n") +
				std::string("Starts a new server of E-Meeting\n") +
				std::string("\n") +
				std::string("  -p             custom port number\n") +
				std::string("  -F             FIFO size\n") +
				std::string("  -L             FIFO low watermark\n") +
				std::string("  -H             FIFO high watermark\n") +
				std::string("  -X             buffer length\n") +
				std::string("  -i             tx interval\n");
		}

		namespace Client {
			const std::string HelpMessage =
				std::string("Usage: ./server [OPTION]...\n") +
				std::string("Starts a new client of E-Meeting\n") +
				std::string("\n") +
				std::string("  -p             port number (optional)\n") +
				std::string("  -s             server name\n") +
				std::string("  -X             retransmit limit\n");
		}
	}
}

#endif // STRINGS_H
