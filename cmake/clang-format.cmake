cmake_minimum_required(VERSION 3.11)

file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.[ch] ${CMAKE_CURRENT_SOURCE_DIR}/src/*.[ch]pp)
file(GLOB_RECURSE TEST_FILES ${CMAKE_CURRENT_SOURCE_DIR}/test/*.[ch] ${CMAKE_CURRENT_SOURCE_DIR}/test/*.[ch]pp)
file(GLOB_RECURSE INCLUDE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/*.[ch] ${CMAKE_CURRENT_SOURCE_DIR}/include/*.[ch]pp)

add_custom_target(clangformat ALL COMMAND
    clang-format-14 -assume-filename=${CMAKE_CURRENT_SOURCE_DIR}/.clang-format -i ${SRCFILES};${TEST_FILES};${INCLUDE_FILES}
)
