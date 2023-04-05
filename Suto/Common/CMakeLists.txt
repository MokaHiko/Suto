cmake_minimum_required(VERSION 3.0.0)
project(AsioCommon VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

file(GLOB SOURCES "include/*.cpp")
file(GLOB HEADERS "include/*.h")
add_library(${PROJECT_NAME} "Suto.h" ${SOURCES} ${HEADERS})
target_include_directories(${PROJECT_NAME} 
	PUBLIC "Z:/Program Files (x86)/asio-1.24.0/include"
	PUBLIC "src")