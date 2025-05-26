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

include_directories(model/include controller/include logger)

set(PROJECT_SOURCES
        main.cpp
        controller/main_controller.cpp
        controller/src/controller_manager.cpp
        controller/models/course_model.cpp
        controller/src/file_input.cpp
        controller/src/course_selection.cpp
        controller/src/schedules_display.cpp
        controller/src/log_display.cpp
        controller/src/ScheduleGenerator.cpp

        controller/include/ScheduleGenerator.h
        controller/include/main_controller.h
        controller/include/controller_manager.h
        controller/models/course_model.h
        controller/include/file_input.h
        controller/include/course_selection.h
        controller/include/schedules_display.h
        controller/include/log_display.h

        model_interfaces.h
        model/main_model.cpp
        logger/logger.cpp
        model/src/parsers/parseCoursesToVector.cpp
        model/src/schedule_algorithm/CourseLegalComb.cpp
        model/src/schedule_algorithm/ScheduleBuilder.cpp
        model/src/schedule_algorithm/TimeUtils.cpp
        model/src/parsers/parseToCsv.cpp
        model/src/parsers/printSchedule.cpp
        model/src/parsers/ScheduleEnrichment.cpp
        model/src/main/model_access.cpp
        model/src/parsers/excel_parser.cpp
        model/src/parsers/excel_parser.cpp
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
        ${CMAKE_CURRENT_SOURCE_DIR}/controller
        ${CMAKE_CURRENT_SOURCE_DIR}/controller/include
        ${CMAKE_CURRENT_SOURCE_DIR}/model
        ${CMAKE_CURRENT_SOURCE_DIR}/model/include
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

# Optional: Copy Excel file to build directory if you have one
# Uncomment and modify the path if you want to include an Excel file with your application
# add_custom_command(TARGET SchedGUI POST_BUILD
#         COMMAND ${CMAKE_COMMAND} -E copy_if_different
#         "${CMAKE_SOURCE_DIR}/EngineeringV2.xlsx"
#         "$<TARGET_FILE_DIR:SchedGUI>/EngineeringV2.xlsx"
#         COMMENT "Copying Excel file to build directory"
# )

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(SchedGUI)
endif()