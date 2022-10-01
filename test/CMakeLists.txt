set(MODULE_NAME test-can-packet)

add_executable(${MODULE_NAME} TestCanPacket.cpp)
target_include_directories(${MODULE_NAME} PUBLIC ../include)
target_link_libraries(${MODULE_NAME} PRIVATE Catch2::Catch2WithMain gcov)

add_test(${MODULE_NAME} ${MODULE_NAME})
