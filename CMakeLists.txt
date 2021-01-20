project(sACN-cpp)
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 14)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

# use this command to use standalone asio
# add_compile_options(-DUSE_STANDALONE_ASIO)
#
# additionally, you will have to add the asio include dirs
# to include_directories, instead of the boost ones

find_package(Boost)
include_directories( {Boost_INCLUDE_DIR} include)

add_executable(example-sender examples/sACN_Sender.cpp)
add_executable(example-receiver examples/sACN_Receiver.cpp)