cmake_minimum_required(VERSION 3.16)

project(SchedGUI VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(COURSEDBINPUT="/data/V1.0CourseDB.txt")

add_compile_definitions(OUTPUTPATH="/data/V1.schedOutput.json")

add_compile_definitions(OUTPUTCOURSEPATH="/data/V1.courseOutput.json")

add_compile_definitions(USERINPUT="/data/userInput.txt")

find_package(Qt6 REQUIRED COMPONENTS Widgets Quick Qml)

include_directories(model/include controller/include)

set(PROJECT_SOURCES
        main.cpp

        controller/main_controller.cpp
        controller/src/CourseModel.cpp

        controller/include/main_controller.h
        controller/include/CourseModel.h

        model/main_model.cpp
        model/src/logs/logger.cpp
        model/src/logs/logManager.cpp
        model/src/parsers/parseCoursesToJson.cpp
        model/src/parsers/parseCoursesToVector.cpp
        model/src/parsers/parseSchedToJson.cpp
        model/src/schedule_algorithm/CourseLegalComb.cpp
        model/src/schedule_algorithm/ScheduleBuilder.cpp
        model/src/schedule_algorithm/TimeUtils.cpp
)

qt_add_resources(RESOURCES view/qml.qrc)

qt_add_executable(SchedGUI
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${RESOURCES}
)

target_include_directories(SchedGUI PRIVATE controller)

target_link_libraries(SchedGUI PRIVATE Qt6::Widgets Qt6::Quick Qt6::Qml)

set_target_properties(SchedGUI PROPERTIES
        ${BUNDLE_ID_OPTION}
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(SchedGUI)
endif()
