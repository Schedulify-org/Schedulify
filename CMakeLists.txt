cmake_minimum_required(VERSION 3.24)
project(Schedulify)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directory for headers
include_directories(include)

add_compile_definitions(COURSEDBINPUT="/data/V1.0CourseDB.txt")

add_compile_definitions(UOTPUTPATH="/data/V1.schedOutput.txt")

add_compile_definitions(USERINPUT="/data/userInput.txt")

# Source files
set(SOURCES
        src/main/main.cpp
        src/main/main_app.cpp
        src/parsers/preParser.cpp
        src/parsers/postParser.cpp
        src/schedule_algorithm/ScheduleBuilder.cpp
        src/schedule_algorithm/CourseLegalComb.cpp
        src/schedule_algorithm/TimeUtils.cpp
        src/logs/logManager.cpp
        src/logs/logger.cpp
)

# Add executable target
add_executable(Schedulify ${SOURCES})
