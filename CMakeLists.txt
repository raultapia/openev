cmake_minimum_required(VERSION 3.15.0)
project(
  openev
  DESCRIPTION "Extending OpenCV to event-based vision"
  HOMEPAGE_URL https://github.com/raultapia/openev)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Git QUIET REQUIRED)
execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

include(FetchContent)
set(BENCHMARK_ENABLE_TESTING OFF)
FetchContent_Declare(google_benchmark GIT_REPOSITORY https://github.com/google/benchmark.git GIT_TAG v1.8.3)
FetchContent_Declare(googletest GIT_REPOSITORY https://github.com/google/googletest.git GIT_TAG v1.17.0)
FetchContent_MakeAvailable(google_benchmark googletest)

# Modules
add_library(openev INTERFACE)
set(OpenEV_LIBRARIES "")
set(BENCHMARK_CMDS "")
foreach(
  MODULE IN
  ITEMS ALGORITHMS
        CONTAINERS
        CORE
        DEVICES
        EVPROC
        READERS
        REPRESENTATIONS)
  string(TOLOWER "${MODULE}" module)
  option(BUILD_MODULE_${MODULE} "Build the ${module} module" ON)
  if(BUILD_MODULE_${MODULE})
    add_subdirectory(modules/${module})
    target_link_libraries(openev INTERFACE oe_${module})
    list(APPEND OpenEV_LIBRARIES "${CMAKE_INSTALL_PREFIX}/lib/openev/liboe_${module}.so")
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/modules/${module}/benchmark")
      list(APPEND BENCHMARK_CMDS COMMAND $<TARGET_FILE:oe_${module}_benchmark>)
    endif()
    set(IS_ENABLED_MODULE_${MODULE} 1)
  else()
    set(IS_ENABLED_MODULE_${MODULE} 0)
  endif()
endforeach()
configure_file("${CMAKE_SOURCE_DIR}/include/openev/options.hpp.in.cmake" "${CMAKE_SOURCE_DIR}/include/openev/options.hpp" @ONLY)
target_include_directories(openev INTERFACE "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>" "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>")
set_target_properties(openev PROPERTIES PUBLIC_HEADER "${PROJECT_SOURCE_DIR}/include/openev/openev.hpp;${PROJECT_SOURCE_DIR}/include/openev/options.hpp")

# Tests
enable_testing()

# Benchmarks
add_custom_target(benchmarks ${BENCHMARK_CMDS})

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
