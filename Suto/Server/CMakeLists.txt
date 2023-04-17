cmake_minimum_required(VERSION 3.0.0)
project(Server VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
file(GLOB SOURCES "src/*.cpp")
file(GLOB HEADERS "src/*.h")

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} AsioCommon)
target_include_directories(${PROJECT_NAME} PUBLIC ../Common/include)