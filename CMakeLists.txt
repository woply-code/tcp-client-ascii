cmake_minimum_required(VERSION 3.16)
project(QtTcpUiNew LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_PREFIX_PATH "E:/Qt/6.11.0/mingw_64")

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

add_executable(${PROJECT_NAME}
    WIN32
    src/main.cpp
    src/TcpClientWindow.cpp
    src/TcpClientWindow.h
    src/TcpClientWindow.ui
)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Network)
