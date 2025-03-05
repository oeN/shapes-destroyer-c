cmake_minimum_required(VERSION 3.30 FATAL_ERROR)

project(my_handmade_hero)

add_library(${PROJECT_NAME} SHARED "my_handmade_hero.cpp")
