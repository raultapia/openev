cmake_minimum_required(VERSION 3.15.0)
project(openev-examples)
set(CMAKE_CXX_STANDARD 17)

add_executable(example-davis example-davis.cpp)
target_link_libraries(example-davis openev)

add_executable(example-event-histogram example-event-histogram.cpp)
target_link_libraries(example-event-histogram openev)

add_executable(example-event-image example-event-image.cpp)
target_link_libraries(example-event-image openev)

add_executable(example-event example-event.cpp)
target_link_libraries(example-event openev)

add_executable(example-plain-text-reader example-plain-text-reader.cpp)
target_link_libraries(example-plain-text-reader openev)

add_executable(example-point-cloud example-point-cloud.cpp)
target_link_libraries(example-point-cloud openev)

add_executable(example-time-surface example-time-surface.cpp)
target_link_libraries(example-time-surface openev)
