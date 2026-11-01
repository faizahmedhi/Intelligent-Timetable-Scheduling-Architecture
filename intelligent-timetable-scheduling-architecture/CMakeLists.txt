cmake_minimum_required(VERSION 3.16)
project(IntelligentTimetableSchedulingArchitecture LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

add_executable(timetable src/main.cpp)
target_link_libraries(timetable PRIVATE Threads::Threads)
