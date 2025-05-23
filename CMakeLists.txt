cmake_minimum_required(VERSION 3.30)
project(db_cpp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt
        GIT_TAG 11.1.4
)

FetchContent_MakeAvailable(fmt)

add_executable(db_cpp main.cpp
        data_types.hpp
        Value.cpp
        Value.hpp
        Column.cpp
        Column.hpp
        Constraint.cpp
        Constraint.hpp
        Table.cpp
        Table.hpp
        Row.cpp
        Row.hpp
        CommonTypes.hpp
        Database.hpp
        Database.cpp
        Command.cpp
        Command.hpp
        Commands.cpp
        Commands.hpp
        Parser.cpp
        Parser.hpp
        Executor.hpp
        Executor.cpp
)
target_link_libraries(db_cpp fmt)
