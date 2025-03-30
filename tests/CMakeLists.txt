cmake_minimum_required(VERSION 3.24)
project(sched_tests)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(USER_DB_PATH="../../data/test_files/mainAppTests.txt")

include_directories(../include ../src ..)

include(FetchContent)
FetchContent_Declare(googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(tests_executable
        main_tests.cpp
        mainAppTests.cpp
        algorithm_test.cpp
        CourseLegalComb_test.cpp
        ../src/main_app.cpp
        ../include/main_app.h
        ../src/algoritm.cpp
        ../src/ScheduleBuilder.cpp
        ../src/CourseLegalComb.cpp
        ../src/parser.cpp
)

target_link_libraries(tests_executable gtest_main)

# Add tests
enable_testing()
add_test(NAME MyTests COMMAND tests_executable)