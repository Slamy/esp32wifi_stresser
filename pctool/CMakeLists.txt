cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(espstresser C CXX)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")


set(Boost_USE_STATIC_LIBS        ON)  # only find static libs
set(Boost_USE_DEBUG_LIBS        OFF)  # ignore debug libs and
set(Boost_USE_RELEASE_LIBS       ON)  # only find release libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
find_package(Boost COMPONENTS date_time filesystem system)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  add_executable(espstresser espstresser.cpp)
  target_link_libraries(espstresser ${Boost_LIBRARIES})
endif()
