project(sACN-cpp)
cmake_minimum_required(VERSION 3.10)

find_package(Boost)
set(CMAKE_CXX_STANDARD 14)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

include_directories( {Boost_INCLUDE_DIR} include)

add_executable(example-sender examples/sACN_Sender.cpp)
add_executable(example-receiver examples/sACN_Receiver.cpp)