cmake_minimum_required(VERSION 3.20.0)
project(openev)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include_directories(include)

find_package(OpenCV REQUIRED)

file(GLOB_RECURSE OPENEV_SRC "${PROJECT_SOURCE_DIR}/src/*")
file(GLOB_RECURSE OPENEV_INC "${PROJECT_SOURCE_DIR}/include/*")
add_library(openev SHARED ${OPENEV_SRC})
target_link_libraries(openev PRIVATE ${OpenCV_LIBRARIES})
set_target_properties(openev PROPERTIES PUBLIC_HEADER "${OPENEV_INC}")

install(TARGETS openev LIBRARY DESTINATION lib PUBLIC_HEADER DESTINATION include/openev)
