set (EMClient_SRCS
	DataBuffer.cpp
	EMClient.cpp
	main.cpp
)

add_executable (client ${EMClient_SRCS})
target_link_libraries (client EMSystem)