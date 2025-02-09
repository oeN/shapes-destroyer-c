cmake_minimum_required(VERSION 3.30 FATAL_ERROR)
project(shapes-destroyer)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_executable(${PROJECT_NAME} src/main.c)
set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 23)

include(cmake/CPM.cmake)

set(LIBS)

CPMAddPackage("gh:libsdl-org/SDL#release-3.2.4")
list(APPEND LIBS SDL3::SDL3)

target_link_libraries(${PROJECT_NAME} ${LIBS})
