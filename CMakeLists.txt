cmake_minimum_required(VERSION 3.16)
project(SimpleCSVReader)

set(CMAKE_CXX_STANDARD 17)

add_executable(SimpleCSVReader main.cpp CSVParser/CSVParser.h CSVParser/CSVParser.cpp)