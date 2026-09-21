set(MODULE_NAME "datasets")
message(STATUS "Module ${MODULE_NAME} included")

file(GLOB_RECURSE SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*")
file(GLOB_RECURSE INC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/openev/${MODULE_NAME}/*")
file(GLOB_RECURSE TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/tests/*")

if(DEFINED BUILD_MODULE_READERS AND NOT BUILD_MODULE_READERS)
  message(FATAL_ERROR "Module ${MODULE_NAME} needs the readers module")
endif()

find_package(OpenCV REQUIRED COMPONENTS core)
find_package(CURL REQUIRED)
find_path(LIBZIP_INCLUDE_DIR zip.h)
find_library(LIBZIP_LIBRARY zip)
if(NOT LIBZIP_INCLUDE_DIR OR NOT LIBZIP_LIBRARY)
  message(FATAL_ERROR "Module ${MODULE_NAME} needs libzip (libzip-dev)")
endif()
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  include(GoogleTest)
endif()

add_library(oe_${MODULE_NAME} SHARED ${SRC_FILES})
target_link_libraries(oe_${MODULE_NAME} PUBLIC opencv_core oe_readers)
target_link_libraries(oe_${MODULE_NAME} PRIVATE CURL::libcurl ${LIBZIP_LIBRARY})
target_include_directories(oe_${MODULE_NAME} PRIVATE ${LIBZIP_INCLUDE_DIR})
target_include_directories(oe_${MODULE_NAME} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>" "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>")
target_include_directories(oe_${MODULE_NAME} PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>" "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>")
set_target_properties(oe_${MODULE_NAME} PROPERTIES PUBLIC_HEADER "${INC_FILES}")

install(
  TARGETS oe_${MODULE_NAME}
  EXPORT OpenEVTargets
  LIBRARY DESTINATION lib/openev
  PUBLIC_HEADER DESTINATION include/openev/${MODULE_NAME})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/openev/${MODULE_NAME}.hpp DESTINATION include/openev)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_executable(oe_${MODULE_NAME}_tests ${TEST_FILES})
  target_link_libraries(oe_${MODULE_NAME}_tests GTest::gtest GTest::gtest_main)
  target_link_libraries(oe_${MODULE_NAME}_tests oe_${MODULE_NAME} ${LIBZIP_LIBRARY})
  target_include_directories(oe_${MODULE_NAME}_tests PRIVATE ${LIBZIP_INCLUDE_DIR})
  gtest_discover_tests(oe_${MODULE_NAME}_tests)
endif()
