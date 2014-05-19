set (EMServer_SRCS
	EMServer.cpp
	main.cpp
)

add_executable (server ${EMServer_SRCS})
target_link_libraries (server EMSystem)