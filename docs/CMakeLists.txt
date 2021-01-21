# see https://devblogs.microsoft.com/cppblog/clear-functional-c-documentation-with-sphinx-breathe-doxygen-cmake/

find_package(Doxygen REQUIRED)
find_package(Sphinx REQUIRED)

# Find all the public headers
file(GLOB_RECURSE SACN_CPP_HEADERS ${SACNCPP_INCLUDE}/*.hpp)

#This will be the main output of our command
set(DOXYGEN_INPUT_DIR ${PROJECT_SOURCE_DIR}/include)
set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/doxygen)
set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/html/index.html)
set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
set(DOXYFILE_OUT ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)

#Replace variables inside @@ with the current values
configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR}) #Doxygen won't create this for us

add_custom_command(OUTPUT ${DOXYGEN_INDEX_FILE}
                   DEPENDS ${SACN_CPP_HEADERS}
                   COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                   MAIN_DEPENDENCY Doxyfile
                   COMMENT "Generating docs")

add_custom_target(doxygen ALL DEPENDS ${DOXYGEN_INDEX_FILE})

set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR})
set(SPHINX_BUILD ${CMAKE_CURRENT_SOURCE_DIR}/sphinx)
set(SPHINX_INDEX_FILE ${SPHINX_BUILD}/index.html)

add_custom_command(OUTPUT ${SPHINX_INDEX_FILE}
                  COMMAND
                  ${SPHINX_EXECUTABLE} -b html
                  -Dbreathe_projects.sACNcpp=${DOXYGEN_OUTPUT_DIR}/xml/
                  ${SPHINX_SOURCE} ${SPHINX_BUILD}
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                  DEPENDS
                   # Other docs files you want to track should go here (or in some variable)
                   ${CMAKE_CURRENT_SOURCE_DIR}/index.rst
                   ${CMAKE_CURRENT_SOURCE_DIR}/sending.rst
                   ${CMAKE_CURRENT_SOURCE_DIR}/receiving.rst
                   ${CMAKE_CURRENT_SOURCE_DIR}/packet.rst
                   ${DOXYGEN_INDEX_FILE}
                   MAIN_DEPENDENCY ${SPHINX_SOURCE}/conf.py
                  COMMENT "Generating documentation with Sphinx")

add_custom_target(sphinx ALL DEPENDS ${SPHINX_INDEX_FILE})