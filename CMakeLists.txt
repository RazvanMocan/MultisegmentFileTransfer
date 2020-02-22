cmake_minimum_required(VERSION 3.15)
project(client C)
project(server C)

set(CMAKE_C_STANDARD 99)

add_executable(client client/main.c)
add_executable(server server/main.c)