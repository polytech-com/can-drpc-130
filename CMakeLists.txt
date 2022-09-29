cmake_minimum_required(VERSION 3.11)
project(can-drpc-130 CXX)

include(FetchContent)

FetchContent_Declare(
    canary
    GIT_REPOSITORY https://github.com/djarek/canary.git
    GIT_TAG 8f2a9612b197552afa3702422fc8521095ffb1bc
)
FetchContent_MakeAvailable(canary)

find_package(Boost 1.70 COMPONENTS program_options REQUIRED)

add_executable(${PROJECT_NAME} src/main.cpp)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} canary ${Boost_LIBRARIES})

if (BUILD_TEST)
    include(CTest)

    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.1.0
    )
    FetchContent_MakeAvailable(Catch2)

    add_subdirectory(test)
endif()
