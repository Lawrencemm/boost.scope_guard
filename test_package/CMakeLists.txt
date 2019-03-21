cmake_minimum_required(VERSION 2.8.12)
project(PackageTest CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(boost_scope_guard REQUIRED)

message(WARNING ${boost_scope_guard_INCLUDES})

add_executable(example example.cpp)
target_link_libraries(example PRIVATE boost_scope_guard::boost_scope_guard)
