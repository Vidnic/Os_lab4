cmake_minimum_required(VERSION 3.10)

project(os_lab4)

set(CMAKE_CXX_STANDARD 17)

add_executable(receiver receiver.cpp)

add_executable(sender sender.cpp)