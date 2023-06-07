cmake_minimum_required(VERSION 3.16)
project(RemoteServer)

# targets and sources
add_executable(server src/server/main.c src/server/logger.c)
add_executable(client src/client/main.c)                                

target_link_libraries(client -pthread)