set(MODULE_NAME "algorithms")
message(STATUS "Module ${MODULE_NAME} included")

find_package(Eigen3 3.4.0 REQUIRED)

file(GLOB_RECURSE SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*")
file(GLOB_RECURSE INC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/openev/${MODULE_NAME}/*")

add_library(efft INTERFACE)
target_include_directories(efft SYSTEM INTERFACE "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/third-party>" "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include/openev>")
install(FILES ${PROJECT_SOURCE_DIR}/third-party/efft/include/efft.hpp DESTINATION include/openev/efft/include/)
install(FILES ${PROJECT_SOURCE_DIR}/third-party/efft/LICENSE DESTINATION include/openev/efft/)

add_library(oe_${MODULE_NAME} SHARED ${SRC_FILES})
target_link_libraries(oe_${MODULE_NAME} PUBLIC ${OpenCV_LIBS})
target_link_libraries(oe_${MODULE_NAME} PUBLIC efft)
target_include_directories(oe_${MODULE_NAME} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>" "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>")
target_include_directories(oe_${MODULE_NAME} PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/modules/core/include>" "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>")
target_include_directories(oe_${MODULE_NAME} PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/modules/utils/include>" "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>")
target_include_directories(oe_${MODULE_NAME} PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>" "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>")
target_include_directories(oe_${MODULE_NAME} PUBLIC "$<BUILD_INTERFACE:${OpenCV_INCLUDE_DIRS}>" "$<INSTALL_INTERFACE:${OpenCV_INCLUDE_DIRS}>")
set_target_properties(oe_${MODULE_NAME} PROPERTIES PUBLIC_HEADER "${INC_FILES}")

install(
  TARGETS oe_${MODULE_NAME} efft
  EXPORT OpenEVTargets
  LIBRARY DESTINATION lib/openev
  PUBLIC_HEADER DESTINATION include/openev/${MODULE_NAME})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/openev/${MODULE_NAME}.hpp DESTINATION include/openev)
