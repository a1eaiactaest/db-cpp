cmake_minimum_required(VERSION 3.30)
project(db_cpp)

set(CMAKE_CXX_STANDARD 20)

inlcude(FetchContent)

FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt
        GIT_TAG 11.1.4
)

FetchContent_MakeAvailable(fmt)

add_executable(db_cpp main.cpp)
target_link_libraries(db_cpp fmt)
