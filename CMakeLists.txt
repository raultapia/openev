cmake_minimum_required(VERSION 3.15.0)
project(
  openev
  DESCRIPTION "Extending OpenCV to event-based vision"
  HOMEPAGE_URL https://github.com/raultapia/openev)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Modules
add_library(openev INTERFACE)
set(OpenEV_LIBRARIES "")
foreach(
  MODULE IN
  ITEMS ALGORITHMS
        CONTAINERS
        CORE
        DEVICES
        READERS
        REPRESENTATIONS
        UTILS)
  string(TOLOWER "${MODULE}" module)
  option(BUILD_MODULE_${MODULE} "Build the ${module} module" ON)
  if(BUILD_MODULE_${MODULE})
    add_subdirectory(modules/${module})
    target_link_libraries(openev INTERFACE oe_${module})
    list(APPEND OpenEV_LIBRARIES "${CMAKE_INSTALL_PREFIX}/lib/openev/liboe_${module}.so")
    set(IS_ENABLED_MODULE_${MODULE} 1)
  else()
    set(IS_ENABLED_MODULE_${MODULE} 0)
  endif()
endforeach()
configure_file("${CMAKE_SOURCE_DIR}/include/openev/options.hpp.in.cmake" "${CMAKE_SOURCE_DIR}/include/openev/options.hpp" @ONLY)
target_include_directories(openev INTERFACE "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>" "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>")
set_target_properties(openev PROPERTIES PUBLIC_HEADER "${PROJECT_SOURCE_DIR}/include/openev/openev.hpp;${PROJECT_SOURCE_DIR}/include/openev/options.hpp")

# Examples
option(BUILD_EXAMPLES "Build the examples" ON)
if(BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

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
set(OpenEV_INCLUDE_DIRS "${CMAKE_INSTALL_PREFIX}/include")
set(OpenEV_LIBRARY_DIRS "${CMAKE_INSTALL_PREFIX}/lib/openev")
configure_package_config_file("OpenEVConfig.cmake.in" "${PROJECT_BINARY_DIR}/OpenEVConfig.cmake" INSTALL_DESTINATION "lib/cmake/openev")
install(FILES "${PROJECT_BINARY_DIR}/OpenEVConfig.cmake" DESTINATION lib/cmake/openev)
