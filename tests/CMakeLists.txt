cmake_minimum_required(VERSION 3.24)
project(sched_tests)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(USER_DB_PATH="../../data/V1.0CourseDB.txt")

include_directories(../include ../src ..)

include(FetchContent)
FetchContent_Declare(googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_executable(tests_executable
        main_tests.cpp
        CourseLegalComb_test.cpp
        TimeUtils_test.cpp
        preParser_test.cpp
        ScheduleBuilder_test.cpp
        ../src/main/main_app.cpp
        ../src/schedule_algorithm/ScheduleBuilder.cpp
        ../src/schedule_algorithm/CourseLegalComb.cpp
        ../src/parsers/preParser.cpp
        ../src/schedule_algorithm/TimeUtils.cpp
        ../src/parsers/postParser.cpp
        postParser_test.cpp
        ../src/logs/logger.cpp
        ../src/logs/logManager.cpp
)

target_link_libraries(tests_executable gtest_main)

# Add tests
enable_testing()
add_test(NAME MyTests COMMAND tests_executable)