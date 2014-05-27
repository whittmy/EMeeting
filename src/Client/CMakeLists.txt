set (EMClient_SRCS
	EMClient.cpp
	main.cpp
)

add_executable (client ${EMClient_SRCS})
target_link_libraries (client EMSystem)