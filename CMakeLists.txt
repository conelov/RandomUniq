cmake_minimum_required(VERSION 3.22)
project(RandomUniq)
set(CMAKE_CXX_STANDARD 20)

add_library(${PROJECT_NAME}_impl INTERFACE
  include/randomUniq/UniformIntDistributionUniq.hpp
  )
target_include_directories(${PROJECT_NAME}_impl INTERFACE
  include
  )

add_executable(${PROJECT_NAME}_example
  example/main.cpp
  )