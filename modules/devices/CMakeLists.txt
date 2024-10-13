set(MODULE_NAME "devices")
message(STATUS "Module ${MODULE_NAME} included")

configure_file("${PROJECT_SOURCE_DIR}/third-party/libcaer/include/libcaer/libcaer.h.in" "${PROJECT_SOURCE_DIR}/third-party/libcaer/include/libcaer/libcaer.h" @ONLY)

add_library(
  caer SHARED
  "${PROJECT_SOURCE_DIR}/third-party/libcaer/src/davis.c"
  "${PROJECT_SOURCE_DIR}/third-party/libcaer/src/device.c"
  "${PROJECT_SOURCE_DIR}/third-party/libcaer/src/dvs128.c"
  "${PROJECT_SOURCE_DIR}/third-party/libcaer/src/dvs132s.c"
  "${PROJECT_SOURCE_DIR}/third-party/libcaer/src/dvxplorer.c"
  "${PROJECT_SOURCE_DIR}/third-party/libcaer/src/dynapse.c"
  "${PROJECT_SOURCE_DIR}/third-party/libcaer/src/filters_dvs_noise.c"
  "${PROJECT_SOURCE_DIR}/third-party/libcaer/src/autoexposure.c"
  "${PROJECT_SOURCE_DIR}/third-party/libcaer/src/frame_utils.c"
  "${PROJECT_SOURCE_DIR}/third-party/libcaer/src/log.c"
  "${PROJECT_SOURCE_DIR}/third-party/libcaer/src/ringbuffer.c"
  "${PROJECT_SOURCE_DIR}/third-party/libcaer/src/samsung_evk.c"
  "${PROJECT_SOURCE_DIR}/third-party/libcaer/src/usb_utils.c"
  "${PROJECT_SOURCE_DIR}/third-party/libusb/libusb/core.c"
  "${PROJECT_SOURCE_DIR}/third-party/libusb/libusb/descriptor.c"
  "${PROJECT_SOURCE_DIR}/third-party/libusb/libusb/hotplug.c"
  "${PROJECT_SOURCE_DIR}/third-party/libusb/libusb/io.c"
  "${PROJECT_SOURCE_DIR}/third-party/libusb/libusb/os/events_posix.c"
  "${PROJECT_SOURCE_DIR}/third-party/libusb/libusb/os/linux_udev.c"
  "${PROJECT_SOURCE_DIR}/third-party/libusb/libusb/os/linux_usbfs.c"
  "${PROJECT_SOURCE_DIR}/third-party/libusb/libusb/os/threads_posix.c"
  "${PROJECT_SOURCE_DIR}/third-party/libusb/libusb/strerror.c"
  "${PROJECT_SOURCE_DIR}/third-party/libusb/libusb/sync.c")
target_link_libraries(caer PRIVATE udev)
target_include_directories(caer PRIVATE "${PROJECT_SOURCE_DIR}/third-party/libcaer/include")
target_include_directories(caer PRIVATE "${PROJECT_SOURCE_DIR}/third-party/libusb/libusb")
target_include_directories(caer PRIVATE "${PROJECT_SOURCE_DIR}/third-party/libusb")
target_compile_options(caer PRIVATE "-w")

file(GLOB_RECURSE SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*")
file(GLOB_RECURSE INC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/openev/${MODULE_NAME}/*")

add_library(oe_${MODULE_NAME} SHARED ${SRC_FILES})
target_link_libraries(oe_${MODULE_NAME} PUBLIC ${OpenCV_LIBS})
target_link_libraries(oe_${MODULE_NAME} PUBLIC caer)
target_include_directories(oe_${MODULE_NAME} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>" "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>")
target_include_directories(oe_${MODULE_NAME} PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/modules/utils/include>" "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>")
target_include_directories(oe_${MODULE_NAME} PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/modules/core/include>" "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>")
target_include_directories(oe_${MODULE_NAME} PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/modules/containers/include>" "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>")
target_include_directories(oe_${MODULE_NAME} PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>" "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>")
target_include_directories(oe_${MODULE_NAME} PUBLIC "$<BUILD_INTERFACE:${OpenCV_INCLUDE_DIRS}>" "$<INSTALL_INTERFACE:${OpenCV_INCLUDE_DIRS}>")
target_include_directories(oe_${MODULE_NAME} SYSTEM PRIVATE "${PROJECT_SOURCE_DIR}/third-party/libcaer/include")
set_target_properties(oe_${MODULE_NAME} PROPERTIES PUBLIC_HEADER "${INC_FILES}")

install(
  TARGETS oe_${MODULE_NAME} caer
  EXPORT OpenEVTargets
  LIBRARY DESTINATION lib/openev
  PUBLIC_HEADER DESTINATION include/openev/${MODULE_NAME})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/openev/${MODULE_NAME}.hpp DESTINATION include/openev)
