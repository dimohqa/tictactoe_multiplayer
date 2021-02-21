cmake_minimum_required(VERSION 3.17)
project(TicTacToe)

set(CMAKE_CXX_STANDARD 14)

add_executable(TicTacToe main.cpp server.cpp Board.cpp Board.h)