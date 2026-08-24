set(MODULE_NAME "readers")
message(STATUS "Module ${MODULE_NAME} included")

file(GLOB_RECURSE SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*")
file(GLOB_RECURSE INC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/openev/${MODULE_NAME}/*")
file(GLOB_RECURSE TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/tests/*")
file(GLOB_RECURSE BENCHMARK_FILES "${CMAKE_CURRENT_SOURCE_DIR}/benchmark/*")

find_package(OpenCV REQUIRED COMPONENTS core highgui calib3d)
find_package(HDF5 REQUIRED COMPONENTS C CXX)
include(GoogleTest)

add_library(oe_${MODULE_NAME} SHARED ${SRC_FILES})
target_link_libraries(oe_${MODULE_NAME} PUBLIC opencv_core opencv_highgui opencv_calib3d)
target_link_libraries(oe_${MODULE_NAME} PUBLIC pthread)
target_link_libraries(oe_${MODULE_NAME} PUBLIC ${HDF5_CXX_LIBRARIES} ${HDF5_LIBRARIES})
target_include_directories(oe_${MODULE_NAME} PUBLIC ${HDF5_INCLUDE_DIRS})
target_include_directories(oe_${MODULE_NAME} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>" "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>")
target_include_directories(oe_${MODULE_NAME} PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/modules/core/include>" "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>")
target_include_directories(oe_${MODULE_NAME} PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/modules/containers/include>" "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>")
target_include_directories(oe_${MODULE_NAME} PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>" "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>")
set_target_properties(oe_${MODULE_NAME} PROPERTIES PUBLIC_HEADER "${INC_FILES}")

install(
  TARGETS oe_${MODULE_NAME}
  EXPORT OpenEVTargets
  LIBRARY DESTINATION lib/openev
  PUBLIC_HEADER DESTINATION include/openev/${MODULE_NAME})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/openev/${MODULE_NAME}.hpp DESTINATION include/openev)

enable_testing()
add_executable(oe_${MODULE_NAME}_tests ${TEST_FILES})
target_link_libraries(oe_${MODULE_NAME}_tests GTest::gtest GTest::gtest_main)
target_link_libraries(oe_${MODULE_NAME}_tests oe_${MODULE_NAME})
gtest_discover_tests(oe_${MODULE_NAME}_tests)

add_executable(oe_${MODULE_NAME}_benchmark ${BENCHMARK_FILES})
target_link_libraries(oe_${MODULE_NAME}_benchmark PRIVATE oe_${MODULE_NAME} benchmark::benchmark benchmark::benchmark_main)
