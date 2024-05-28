cmake_minimum_required(VERSION 3.28)
project(VANET)

set(CMAKE_CXX_STANDARD 17)

include_directories()
add_executable(VANET src/main.cpp
        src/MacLayer.cpp
        src/PhysicLayer.cpp
        src/World.cpp
)
target_link_libraries(VANET PUBLIC tracicpp)
