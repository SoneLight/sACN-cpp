project(sACN-cpp)
cmake_minimum_required(VERSION 3.10)

find_package(Boost)
set(CMAKE_CXX_STANDARD 14)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

add_executable(example-sender examples/sACN_Sender.cpp)
target_include_directories(example-sender PRIVATE ${Boost_INCLUDE_DIR} include)