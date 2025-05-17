cmake_minimum_required(VERSION 3.24)
project(schedModelTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

add_compile_definitions(USER_DB_PATH="../../data/V1.0CourseDB.txt")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Quick Qml QuickLayouts PrintSupport)

include_directories(../../model/include ../../model/src ../logger ../..)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

include(FetchContent)
FetchContent_Declare(googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(schedModelTest
        ../../logger/logger.cpp
        ../../logger/logger.h

        ../../model/src/parsers/parseCoursesToJson.cpp
        ../../model/src/parsers/parseCoursesToVector.cpp
        ../../model/src/schedule_algorithm/CourseLegalComb.cpp
        ../../model/src/schedule_algorithm/ScheduleBuilder.cpp
        ../../model/src/schedule_algorithm/TimeUtils.cpp
        ../../model/src/parsers/parseToCsv.cpp
        ../../model/src/parsers/printSchedule.cpp
        ../../model/src/parsers/ScheduleEnrichment.cpp
        ../../model/src/main/model_access.cpp
        ../../model/main_model.cpp

        main_tests.cpp
        CourseLegalComb_test.cpp
        TimeUtils_test.cpp
        preParser_test.cpp
        ScheduleBuilder_test.cpp
        scheduleEnrichment_test.cpp
)

# Link Qt libraries
target_link_libraries(schedModelTest
        PRIVATE
        gtest_main
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Quick
        Qt6::Qml
        Qt6::QuickLayouts
        Qt6::PrintSupport
)

# Add model-tests
enable_testing()
add_test(NAME MyTests COMMAND schedModelTest)