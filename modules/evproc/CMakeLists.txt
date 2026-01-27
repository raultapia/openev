set(MODULE_NAME "evproc")
message(STATUS "Module ${MODULE_NAME} included")

file(GLOB_RECURSE SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*")
file(GLOB_RECURSE INC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/openev/${MODULE_NAME}/*")

find_package(OpenCV REQUIRED COMPONENTS core highgui calib3d)

add_library(oe_${MODULE_NAME} SHARED ${SRC_FILES})
target_link_libraries(oe_${MODULE_NAME} PUBLIC opencv_core opencv_highgui opencv_calib3d)
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
