file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false *.hpp *.cpp)

set(SOURCES ${TEST_SOURCES})

message("TEST_SOURCES: ${TEST_SOURCES}")

add_executable(sacncpptests ${TEST_SOURCES})

add_test(NAME sacncpptests COMMAND sacncpptests)

target_link_libraries(sacncpptests PUBLIC gtest)