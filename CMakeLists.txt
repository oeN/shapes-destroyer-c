set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")
cmake_minimum_required(VERSION 3.30 FATAL_ERROR)

project(main)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# for debug symbols
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -DINTERNAL_BUILD")

file(GLOB SRC_SOURCES "src/sdl_main.cpp") # trying unity build
add_executable(${PROJECT_NAME} ${SRC_SOURCES})

set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 23)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)


include(cmake/CPM.cmake)

set(LIBS)

CPMAddPackage("gh:libsdl-org/SDL#release-3.2.4")
list(APPEND LIBS SDL3::SDL3)

target_link_libraries(${PROJECT_NAME} ${LIBS})

add_subdirectory("src/shapes_destroyer")
add_subdirectory("src/my_handmade_hero")

# add_subdirectory(tests)
