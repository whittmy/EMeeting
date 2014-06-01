project (EMeeting)
cmake_minimum_required (VERSION 2.8)
set (CMAKE_CXX_FLAGS "-Wall -std=c++11 -lboost_system -lpthread")

find_package(Boost 1.53.0 REQUIRED COMPONENTS thread system)

include_directories (${EMeeting_SOURCE_DIR}/src)
set (EXECUTABLE_OUTPUT_PATH "${EMeeting_BINARY_DIR}/bin")
set (LIBRARY_OUTPUT_PATH "${EMeeting_BINARY_DIR}/lib")

add_subdirectory (src)

install (DIRECTORY data DESTINATION "${EMeeting_BINARY_DIR}")
