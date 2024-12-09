set(MODULE_NAME "containers")
message(STATUS "Module ${MODULE_NAME} included")

file(GLOB_RECURSE SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*")
file(GLOB_RECURSE INC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/openev/${MODULE_NAME}/*")

add_library(boost INTERFACE)
target_include_directories(boost SYSTEM INTERFACE "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/third-party/boost/assert/include>" "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include/openev>")
target_include_directories(boost SYSTEM INTERFACE "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/third-party/boost/circular_buffer/include>" "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include/openev>")
target_include_directories(boost SYSTEM INTERFACE "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/third-party/boost/concept_check/include>" "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include/openev>")
target_include_directories(boost SYSTEM INTERFACE "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/third-party/boost/config/include>" "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include/openev>")
target_include_directories(boost SYSTEM INTERFACE "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/third-party/boost/core/include>" "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include/openev>")
target_include_directories(boost SYSTEM INTERFACE "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/third-party/boost/move/include>" "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include/openev>")
target_include_directories(boost SYSTEM INTERFACE "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/third-party/boost/preprocessor/include>" "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include/openev>")
target_include_directories(boost SYSTEM INTERFACE "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/third-party/boost/static_assert/include>" "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include/openev>")
target_include_directories(boost SYSTEM INTERFACE "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/third-party/boost/throw_exception/include>" "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include/openev>")
target_include_directories(boost SYSTEM INTERFACE "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/third-party/boost/type_traits/include>" "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include/openev>")
install(DIRECTORY ${PROJECT_SOURCE_DIR}/third-party/boost/assert/include/boost DESTINATION include/openev)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/third-party/boost/circular_buffer/include/boost DESTINATION include/openev)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/third-party/boost/concept_check/include/boost DESTINATION include/openev)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/third-party/boost/config/include/boost DESTINATION include/openev)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/third-party/boost/core/include/boost DESTINATION include/openev)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/third-party/boost/move/include/boost DESTINATION include/openev)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/third-party/boost/preprocessor/include/boost DESTINATION include/openev)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/third-party/boost/static_assert/include/boost DESTINATION include/openev)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/third-party/boost/throw_exception/include/boost DESTINATION include/openev)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/third-party/boost/type_traits/include/boost DESTINATION include/openev)

add_library(oe_${MODULE_NAME} SHARED ${SRC_FILES})
target_link_libraries(oe_${MODULE_NAME} PUBLIC ${OpenCV_LIBS})
target_link_libraries(oe_${MODULE_NAME} PUBLIC boost)
target_include_directories(oe_${MODULE_NAME} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>" "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>")
target_include_directories(oe_${MODULE_NAME} PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/modules/core/include>" "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>")
target_include_directories(oe_${MODULE_NAME} PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/modules/utils/include>" "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>")
target_include_directories(oe_${MODULE_NAME} PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>" "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>")
target_include_directories(oe_${MODULE_NAME} PUBLIC "$<BUILD_INTERFACE:${OpenCV_INCLUDE_DIRS}>" "$<INSTALL_INTERFACE:${OpenCV_INCLUDE_DIRS}>")
set_target_properties(oe_${MODULE_NAME} PROPERTIES PUBLIC_HEADER "${INC_FILES}")

install(
  TARGETS oe_${MODULE_NAME} boost
  EXPORT OpenEVTargets
  LIBRARY DESTINATION lib/openev
  PUBLIC_HEADER DESTINATION include/openev/${MODULE_NAME})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/openev/${MODULE_NAME}.hpp DESTINATION include/openev)
