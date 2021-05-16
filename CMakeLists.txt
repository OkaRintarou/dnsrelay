cmake_minimum_required(VERSION 3.17)
project(dnsrelay C)

set(CMAKE_C_STANDARD 99)

link_libraries(ws2_32)

add_executable(dnsrelay main.c handling.h handling.c arrayList.c arrayList.h)
add_executable(test test.c arrayList.c arrayList.h)