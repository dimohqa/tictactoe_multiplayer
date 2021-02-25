cmake_minimum_required(VERSION 3.17)
project(TicTacToe)
find_library(PTHREAD_LIBRARY pthread)

set(CMAKE_CXX_STANDARD 14)

add_executable(Server server.cpp Board.cpp Board.h Status.cpp Status.h Player.cpp Player.h Game.cpp Game.h sockets.cpp sockets.h)

add_executable(Client1 client.cpp Board.cpp Board.h Status.cpp Status.h Player.cpp Player.h sockets.cpp sockets.h)

add_executable(Client2 client.cpp Board.cpp Board.h Status.cpp Status.h Player.cpp Player.h sockets.cpp sockets.h)

target_link_libraries(Server ${PTHREAD_LIBRARY})