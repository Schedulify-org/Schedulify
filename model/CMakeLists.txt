cmake_minimum_required(VERSION 3.16)
project(Schedulify VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directory for headers
include_directories(include)

add_compile_definitions(COURSEDBINPUT="/data/V1.0CourseDB.txt")

add_compile_definitions(OUTPUTPATH="/data/V1.schedOutput.json")

add_compile_definitions(OUTPUTCOURSEPATH="/data/V1.courseOutput.json")

add_compile_definitions(USERINPUT="/data/userInput.txt")

# Source files
set(SOURCES
        src/parsers/parseCoursesToVector.cpp
        src/schedule_algorithm/ScheduleBuilder.cpp
        src/schedule_algorithm/CourseLegalComb.cpp
        src/schedule_algorithm/TimeUtils.cpp
        ../logger/logger.cpp
        src/parsers/parseCoursesToJson.cpp
)

# Add executable target
add_executable(Schedulify ${SOURCES})
