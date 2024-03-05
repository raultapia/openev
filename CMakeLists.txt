cmake_minimum_required(VERSION 3.15.0)
project(
  openev
  DESCRIPTION "Extending OpenCV to event-based vision"
  HOMEPAGE_URL https://github.com/raultapia/openev)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(OpenCV REQUIRED)

# Add modules
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

include(CMakePackageConfigHelpers)
set(OpenEV_LIBRARIES "${CMAKE_INSTALL_PREFIX}/lib/openev")
set(OpenEV_INCLUDE_DIRS "${CMAKE_INSTALL_PREFIX}/include/openev")
configure_package_config_file("OpenEVConfig.cmake.in" "${PROJECT_BINARY_DIR}/OpenEVConfig.cmake" INSTALL_DESTINATION "lib/cmake/openev")
install(FILES "${PROJECT_BINARY_DIR}/OpenEVConfig.cmake" DESTINATION lib/cmake/openev)
