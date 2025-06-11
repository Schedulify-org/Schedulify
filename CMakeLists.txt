cmake_minimum_required(VERSION 3.16)

project(SchedGUI VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0")
endif()

# Fetch OpenXLSX for Excel file parsing
include(FetchContent)
FetchContent_Declare(
        OpenXLSX
        GIT_REPOSITORY https://github.com/troldal/OpenXLSX.git
        GIT_TAG        master
        GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(OpenXLSX)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Quick Qml QuickLayouts PrintSupport QuickControls2)

set(PROJECT_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/model_interfaces.h

        ${CMAKE_CURRENT_SOURCE_DIR}/controller/include/main_controller.h
        ${CMAKE_CURRENT_SOURCE_DIR}/controller/include/controller_manager.h
        ${CMAKE_CURRENT_SOURCE_DIR}/controller/include/file_input.h
        ${CMAKE_CURRENT_SOURCE_DIR}/controller/include/course_selection.h
        ${CMAKE_CURRENT_SOURCE_DIR}/controller/include/schedules_display.h
        ${CMAKE_CURRENT_SOURCE_DIR}/controller/include/log_display.h
        ${CMAKE_CURRENT_SOURCE_DIR}/controller/adapters/thread_workers/ScheduleGenerator.h
        ${CMAKE_CURRENT_SOURCE_DIR}/controller/adapters/thread_workers/CourseValidator.h

        ${CMAKE_CURRENT_SOURCE_DIR}/controller/adapters/view_models/course_model.h
        ${CMAKE_CURRENT_SOURCE_DIR}/controller/adapters/view_models/schedule_model.h

        ${CMAKE_CURRENT_SOURCE_DIR}/controller/adapters/filters/schedule_filter.h

        ${CMAKE_CURRENT_SOURCE_DIR}/controller/main_controller.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/controller/src/controller_manager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/controller/src/file_input.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/controller/src/course_selection.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/controller/src/schedules_display.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/controller/src/log_display.cpp
        controller/adapters/thread_workers/ScheduleGenerator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/controller/adapters/filters/schedule_filter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/controller/adapters/view_models/course_model.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/controller/adapters/view_models/schedule_model.cpp
        controller/adapters/thread_workers/CourseValidator.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/logger/logger.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/model/main_model.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/model/src/main/model_access.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/model/src/parsers/parseCoursesToVector.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/model/src/parsers/parseToCsv.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/model/src/parsers/printSchedule.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/model/src/parsers/excel_parser.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/model/src/parsers/validate_courses.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/model/src/schedule_algorithm/CourseLegalComb.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/model/src/schedule_algorithm/ScheduleBuilder.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/model/src/schedule_algorithm/TimeUtils.cpp

)

qt_add_resources(RESOURCES view/qml.qrc)

qt_add_executable(SchedGUI
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${RESOURCES}
)

# Include directories
target_include_directories(SchedGUI PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/model/include/main
        ${CMAKE_CURRENT_SOURCE_DIR}/model/include/parsers
        ${CMAKE_CURRENT_SOURCE_DIR}/model/include/schedule_algorithm
        ${CMAKE_CURRENT_SOURCE_DIR}/controller/include
        ${CMAKE_CURRENT_SOURCE_DIR}/controller/adapters/view_models
        ${CMAKE_CURRENT_SOURCE_DIR}/controller/adapters/filters
        ${CMAKE_CURRENT_SOURCE_DIR}/controller/adapters/thread_workers
        ${CMAKE_CURRENT_SOURCE_DIR}/logger
)

# Link Qt libraries and OpenXLSX
target_link_libraries(SchedGUI PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Quick
        Qt6::Qml
        Qt6::QuickLayouts
        Qt6::PrintSupport
        Qt6::QuickControls2
        OpenXLSX::OpenXLSX
)

# Enable UTF-8 support on Windows for Excel file handling
if(WIN32)
    if(MSVC)
        target_compile_options(SchedGUI PRIVATE /utf-8)
    else()
        # For MinGW/GCC
        target_compile_options(SchedGUI PRIVATE -finput-charset=utf-8 -fexec-charset=utf-8)
    endif()
endif()

set_target_properties(SchedGUI PROPERTIES
        ${BUNDLE_ID_OPTION}
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE FALSE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(SchedGUI)
endif()