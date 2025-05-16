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
        data_types.h
        Value.cpp
        Value.h
        Column.cpp
        Column.h
        Constraint.cpp
        Constraint.h
        Table.cpp
        Table.h
        Row.cpp
        Row.h
        CommonTypes.h
        Database.h
        Database.cpp
        Command.cpp
        Command.hpp
        Commands.cpp
        Commands.hpp
        Parser.cpp
        Parser.hpp
        Executor.hpp
        Executor.cpp
        Serializer.h
)
target_link_libraries(db_cpp fmt)
