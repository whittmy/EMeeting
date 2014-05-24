set (EMServer_SRCS
	ClientObject.cpp
	EMServer.cpp
	main.cpp
	TcpConnection.cpp
)

add_executable (server ${EMServer_SRCS})
target_link_libraries (server EMSystem)