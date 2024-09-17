cmake_minimum_required(VERSION 3.24)

project(TestClient)

add_executable(TestClient main.cpp)
target_link_libraries(TestClient PRIVATE Utils)