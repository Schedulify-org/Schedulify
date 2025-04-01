cmake_minimum_required(VERSION 3.24)
project(Schedulify)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directory for headers
include_directories(include)

# Source files
set(SOURCES
    src/main.cpp
    src/main_app.cpp
    src/parser.cpp
    src/ScheduleBuilder.cpp
    src/CourseLegalComb.cpp
    src/TimeUtils.cpp
)

# Add executable target
add_executable(Schedulify ${SOURCES})
