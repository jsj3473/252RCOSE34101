# [WARNING] DO NOT MODIFY THIS FILE
cmake_minimum_required(VERSION 3.10)
project(kudb)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUNIX -Wall -g")

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR})


# do not add/remove c++ files
file(GLOB SRCS ${PROJECT_SOURCE_DIR}/src/*.cpp)

add_executable(lock ${SRCS})
target_compile_definitions(lock PRIVATE LOCK)

add_executable(occ  ${SRCS})
target_compile_definitions(occ PRIVATE OCC)
#add_executable(kudb ${SRCS})
