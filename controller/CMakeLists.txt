cmake_minimum_required(VERSION 3.16)

project(SchedGUI VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Quick Qml)

set(PROJECT_SOURCES
        main.cpp
        src/Controller.cpp
        include/Controller.h
)

qt_add_resources(RESOURCES ../view/qml.qrc)

qt_add_executable(SchedGUI
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${RESOURCES}
)

target_include_directories(SchedGUI PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

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
