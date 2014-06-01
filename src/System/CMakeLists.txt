set (EMSystem_SRCS
	AbstractServer.cpp
	ArgsManager.cpp
	Error.cpp
	Messages.cpp
	SignalHandler.cpp
)

add_library (EMSystem ${EMSystem_SRCS})
