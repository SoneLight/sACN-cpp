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
set(SACNCPP_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories( ${Boost_INCLUDE_DIR} ${SACNCPP_INCLUDE})

add_executable(example-sender examples/sACN_Sender.cpp)
add_executable(example-receiver examples/sACN_Receiver.cpp)

# Add the cmake folder so the FindSphinx module is found
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

add_subdirectory(docs)