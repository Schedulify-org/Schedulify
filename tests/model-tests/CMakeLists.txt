cmake_minimum_required(VERSION 3.24)
project(schedModelTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

add_compile_definitions(USER_DB_PATH="../../data/V1.0CourseDB.txt")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Quick Qml QuickLayouts PrintSupport)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

include(FetchContent)

# Fetch OpenXLSX for Excel file parsing
FetchContent_Declare(
        OpenXLSX
        GIT_REPOSITORY https://github.com/troldal/OpenXLSX.git
        GIT_TAG        master
        GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(OpenXLSX)

# Fetch GoogleTest
FetchContent_Declare(googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(schedModelTest
        ${CMAKE_CURRENT_SOURCE_DIR}/../../logger/logger.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../logger/logger.h

        ${CMAKE_CURRENT_SOURCE_DIR}/../../model/src/parsers/parseCoursesToVector.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../model/src/parsers/excel_parser.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../model/src/schedule_algorithm/CourseLegalComb.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../model/src/schedule_algorithm/ScheduleBuilder.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../model/src/parsers/validate_courses.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../model/src/schedule_algorithm/TimeUtils.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../model/src/parsers/parseToCsv.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../model/src/parsers/printSchedule.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../model/src/main/model_access.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../model/main_model.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/CourseLegalComb_test.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/main_tests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/CourseLegalComb_test.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/TimeUtils_test.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/preParser_test.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ScheduleBuilder_test.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/scheduleEnrichment_test.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/excel_parser_test.cpp
)

# Use target_include_directories instead of include_directories
target_include_directories(schedModelTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../..
        ${CMAKE_CURRENT_SOURCE_DIR}/../../model/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../model/include/main
        ${CMAKE_CURRENT_SOURCE_DIR}/../../model/include/parsers
        ${CMAKE_CURRENT_SOURCE_DIR}/../../model/include/schedule_algorithm
        ${CMAKE_CURRENT_SOURCE_DIR}/../../logger
)

# Link Qt libraries and OpenXLSX
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
        OpenXLSX::OpenXLSX
)

# Add model-tests
enable_testing()
add_test(NAME MyTests COMMAND schedModelTest)