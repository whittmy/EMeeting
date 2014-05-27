set (EMSystem_SRCS
	AbstractServer.cpp
	ArgsManager.cpp
	DataBuffer.cpp
	Error.cpp
	Messages.cpp
	Mixer.cpp
	SignalHandler.cpp
)

add_library (EMSystem ${EMSystem_SRCS})
