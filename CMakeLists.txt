cmake_minimum_required(VERSION 3.24)
project(Schedulify)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(SCHED
        src/main.cpp
        src/main_app.cpp)