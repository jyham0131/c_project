cmake_minimum_required(VERSION 4.2)
project(CLion C)

set(CMAKE_C_STANDARD 11)

# Windows Sockets 2 library linking
link_libraries(ws2_32)

add_executable(ChatServer server.c)
add_executable(ChatClient client.c)