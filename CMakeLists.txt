cmake_minimum_required(VERSION 3.15.0)
project(
  openev
  DESCRIPTION "Extending OpenCV to event-based vision"
  HOMEPAGE_URL https://github.com/raultapia/openev)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Git submodules
execute_process(
  COMMAND id -u
  OUTPUT_VARIABLE USER_ID
  OUTPUT_STRIP_TRAILING_WHITESPACE)
if(NOT "${USER_ID}" STREQUAL "0")
  find_package(Git REQUIRED)
  execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} COMMAND_ERROR_IS_FATAL ANY)
endif()

# Add modules
find_package(OpenCV REQUIRED)
add_subdirectory(modules/containers)
add_subdirectory(modules/core)
add_subdirectory(modules/devices)
add_subdirectory(modules/readers)
add_subdirectory(modules/representations)
add_subdirectory(modules/utils)

# Compile
add_library(openev INTERFACE)
target_link_libraries(openev INTERFACE oe_containers)
target_link_libraries(openev INTERFACE oe_core)
target_link_libraries(openev INTERFACE oe_devices)
target_link_libraries(openev INTERFACE oe_readers)
target_link_libraries(openev INTERFACE oe_representations)
target_link_libraries(openev INTERFACE oe_utils)
target_include_directories(openev INTERFACE "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>" "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>")
set_target_properties(openev PROPERTIES PUBLIC_HEADER "${PROJECT_SOURCE_DIR}/include/openev/openev.hpp")

# Compile examples
add_subdirectory(examples)

# Install
install(
  TARGETS openev
  EXPORT OpenEVTargets
  PUBLIC_HEADER DESTINATION include/openev)

install(
  EXPORT OpenEVTargets
  FILE OpenEVTargets.cmake
  DESTINATION lib/cmake/openev)

