cmake_minimum_required(VERSION 3.30 FATAL_ERROR)

project(tests)

file(GLOB SRC_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c" "${CMAKE_SOURCE_DIR}/src/*.c")
list(REMOVE_ITEM SRC_SOURCES "${CMAKE_SOURCE_DIR}/src/main.c")

add_executable(${PROJECT_NAME} ${SRC_SOURCES})
set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 23)

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/src")

include(../cmake/CPM.cmake)

set(LIBS)

CPMAddPackage("gh:libsdl-org/SDL#release-3.2.4")
list(APPEND LIBS SDL3::SDL3)
CPMAddPackage("gh:ThrowTheSwitch/Unity#master")
list(APPEND LIBS unity::framework)

target_link_libraries(${PROJECT_NAME} ${LIBS})

link_directories(CMAKE_CURRENT_SOURCE_DIR)
