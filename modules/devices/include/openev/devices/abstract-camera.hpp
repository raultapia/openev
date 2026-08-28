/*!
\file abstract-camera.hpp
\brief Abstract camera device driver.
\author Raul Tapia
*/
#ifndef OPENEV_DEVICES_ABSTRACT_CAMERA_HPP
#define OPENEV_DEVICES_ABSTRACT_CAMERA_HPP

#include "openev/containers/queue.hpp"
#include "openev/containers/vector.hpp"
#include "openev/core/types.hpp"
#include <atomic>
#include <cstddef>
#include <math.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/mat.inl.hpp>
#include <opencv2/core/types.hpp>
#include <ostream>
#include <queue>
#include <stdint.h>
#include <string>
#include <vector>

typedef struct caer_device_handle {
  uint16_t deviceType;
} *caerDeviceHandle;

namespace ev {

namespace unit {
template <typename T>
constexpr double us(const T x) { return static_cast<double>(x) * 1e-6; }
} // namespace unit

constexpr double EARTH_GRAVITY = 9.80665;
constexpr double DEG2RAD = M_PI / 180.0;
constexpr double SCALE_16B_8B = 1.0 / 256.0;

/*!
\brief This class extends cv::Mat to include timestamp.

The following aliases are defined for convenience:
\code{.cpp}
using StampedMatVector = std::vector<StampedMat>;
using StampedMatQueue = std::queue<StampedMat>;
\endcode
*/
class StampedMat : public cv::Mat {
public:
  TimeType t{0};

  using cv::Mat::copyTo;

  /*!
  \brief Copy image data and timestamp to another StampedMat.
  \param dst Destination
  */
  void copyTo(StampedMat &dst) const {
    cv::Mat::copyTo(dst);
    dst.t = t;
  }

  /*!
  \brief Release the image data and reset the timestamp.
  */
  void release() {
    cv::Mat::release();
    t = 0;
  }
};
using StampedMatVector = std::vector<StampedMat>;
using StampedMatQueue = std::queue<StampedMat>;

/*!
\brief This struct is used to store linear acceleration and angular velocity.
*/
struct xyz_t {
  double x{0};
  double y{0};
  double z{0};

  [[nodiscard]] bool empty() const {
    return x == 0 && y == 0 && z == 0;
  }

  void release() {
    x = y = z = 0;
  }

  friend std::ostream &operator<<(std::ostream &os, const xyz_t &xyz) {
    os << "(" << xyz.x << ", " << xyz.y << ", " << xyz.z << ")";
    return os;
  }
};

/*!
\brief This struct is used to store IMU data from a DAVIS event camera.

The following aliases are defined for convenience:
\code{.cpp}
using ImuVector = std::vector<Imu>;
using ImuQueue = std::queue<Imu>;
\endcode
*/
struct Imu {
  TimeType t{0};
  xyz_t linear_acceleration;
  xyz_t angular_velocity;

  [[nodiscard]] bool empty() const {
    return t == 0 && linear_acceleration.empty() && angular_velocity.empty();
  }

  void release() {
    t = 0;
    linear_acceleration.release();
    angular_velocity.release();
  }

  friend std::ostream &operator<<(std::ostream &os, const Imu &imu) {
    os << "t: " << imu.t << ", acc: " << imu.linear_acceleration << ", gyr: " << imu.angular_velocity;
    return os;
  }
};
using ImuVector = std::vector<Imu>;
using ImuQueue = std::queue<Imu>;

/*!
\brief This is an auxiliary class. This class cannot be instanced.
*/
class AbstractCamera {
public:
  /*! \cond INTERNAL */
  AbstractCamera() = default;
  virtual ~AbstractCamera();
  AbstractCamera(const AbstractCamera &) = delete;
  AbstractCamera(AbstractCamera &&) noexcept = delete;
  AbstractCamera &operator=(const AbstractCamera &) = delete;
  AbstractCamera &operator=(AbstractCamera &&) noexcept = delete;
  /*! \endcond */

  /*!
  \brief Start reading data.
  */
  virtual void start() = 0;

  /*!
  \brief Stop reading data.
  */
  void stop();

  /*!
  \brief Check whether the device was found and opened.
  \return True if the camera is open
  */
  [[nodiscard]] bool isOpen() const {
    return deviceHandler_ != nullptr;
  }

  /*!
  \brief Check whether the device produces APS frames.
  \return True if the camera has an APS sensor
  */
  [[nodiscard]] virtual bool hasAps() const = 0;

  /*!
  \brief Check whether the device produces IMU data.
  \return True if the camera has an IMU
  */
  [[nodiscard]] virtual bool hasImu() const = 0;

  /*!
  \brief Get device sensor size.
  \return Sensor size
  */
  [[nodiscard]] virtual cv::Size getSensorSize() const = 0;

  /*!
  \brief Get device serial number, prefixed with the camera family.
  \return Serial number, empty if the camera is not open
  */
  [[nodiscard]] virtual std::string getSerialNumber() const = 0;

  /*!
  \brief Get device reset time.
  \return Reset time in microseconds
  */
  [[nodiscard]] uint64_t getResetTime() const {
    return resetTime_;
  }

  /*!
  \brief Get current ROI.
  \return ROI
  */
  [[nodiscard]] cv::Rect_<uint16_t> getRoi() const;

  /*!
  \brief Set current ROI. Events outside the ROI are not considered. Images are cropped according to the ROI.
  \param roi ROI
  \return True if valid ROI
  */
  virtual bool setRoi(const cv::Rect_<uint16_t> &roi) = 0;

  /*!
  \brief Load a defective pixel file. Events on hot pixels are discarded, and saturated pixels are replaced by the median of their neighbours on every frame.
  \param defective_pixels_file Path to a YAML file
  \note The file holds one entry per camera, keyed by the serial number reported by getSerialNumber(). Each entry lists the defective pixels as flat sequences of x, y pairs. Every event at a hot_pixels coordinate is discarded. Every saturated_pixels coordinate is replaced on each APS frame by the median of its neighbours, excluding those that are themselves saturated or fall outside the frame. The dead_pixels entry may be present but is not used.
  \code{.yaml}
  %YAML:1.0
  ---
  DAVIS346B-00000259:
     dead_pixels: []
     hot_pixels: [ 102, 216, 308, 205 ]
     saturated_pixels: [ 183, 126 ]
  DAVIS240C-00000117:
     dead_pixels: []
     hot_pixels: [ 5, 5 ]
     saturated_pixels: []
  \endcode
  */
  void setDefectivePixels(const std::string &defective_pixels_file);

  /*!
  \brief Set the maximum time interval between subsequent containers.
  \param usec Maximum time interval in microseconds
  */
  void setContainerInterval(const uint32_t usec);

  /*!
  \brief Set the maximum number of events a container may hold before it is delivered. The limit applies to each typed packet of the container separately, so a container carrying
  events, frames, and imu data can deliver up to this number of each of them.
  \warning Set to zero to disable.
  \param n Maximum number of events
  */
  void setContainerSize(const uint32_t n);

  /*!
  \brief Get DVS data.
  \param events Event vector to which events will be added
  \return True if vector not empty
  */
  bool getData(Vector &events);

  /*!
  \brief Get DVS data.
  \param events Event queue to which events will be added.
  \return True if queue not empty
  */
  bool getData(Queue &events);

  /*!
  \brief Get APS data.
  \param frame Frame destination
  \return True if frame not empty
  */
  bool getData(StampedMat &frame);

  /*!
  \brief Get APS data.
  \param frames Frame vector to which frame will be added
  \return True if vector not empty
  */
  bool getData(StampedMatVector &frames);

  /*!
  \brief Get APS data.
  \param frames Frame queue to which frame will be added
  \return True if queue not empty
  */
  bool getData(StampedMatQueue &frames);

  /*!
  \brief Get IMU data.
  \param imu Imu data destination
  \return True if imu data not empty
  */
  bool getData(Imu &imu);

  /*!
  \brief Get IMU data.
  \param imu Imu data vector to which imu data will be added
  \return True if vector not empty
  */
  bool getData(ImuVector &imu);

  /*!
  \brief Get IMU data.
  \param imu Imu data queue to which imu data will be added
  \return True if queue not empty
  */
  bool getData(ImuQueue &imu);

  /*!
  \brief Get DVS+APS data.
  \param events Event vector to which events will be added
  \param frame Frame destination
  \return True if event vector or frame not empty
  */
  bool getData(Vector &events, StampedMat &frame);

  /*!
  \brief Get DVS+APS data.
  \param events Event vector to which events will be added
  \param frames Frame vector to which frame will be added
  \return True if event vector or frame vector not empty
  */
  bool getData(Vector &events, StampedMatVector &frames);

  /*!
  \brief Get DVS+APS data.
  \param events Event queue to which events will be added
  \param frames Frame queue to which frame will be added
  \return True if event queue or frame queue not empty
  */
  bool getData(Queue &events, StampedMatQueue &frames);

  /*!
  \brief Get DVS+IMU data.
  \param events Event vector to which events will be added
  \param imu Imu data destination
  \return True if event vector or imu data not empty
  */
  bool getData(Vector &events, Imu &imu);

  /*!
  \brief Get DVS+IMU data.
  \param events Event vector to which events will be added
  \param imu Imu data vector to which imu data will be added
  \return True if event vector or imu data vector not empty
  */
  bool getData(Vector &events, ImuVector &imu);

  /*!
  \brief Get DVS+IMU data.
  \param events Event queue to which events will be added
  \param imu Imu data queue to which imu data will be added
  \return True if event queue or imu data queue not empty
  */
  bool getData(Queue &events, ImuQueue &imu);

  /*!
  \brief Get DVS+APS+IMU data.
  \param events Event vector to which events will be added
  \param frame Frame destination
  \param imu Imu data destination
  \return True if event vector, frame, or imu data not empty
  */
  bool getData(Vector &events, StampedMat &frame, Imu &imu);

  /*!
  \brief Get DVS+APS+IMU data.
  \param events Event vector to which events will be added
  \param frames Frame vector to which frame will be added
  \param imu Imu data vector to which imu data will be added
  \return True if event vector, frame vector, or imu data vector not empty
  */
  bool getData(Vector &events, StampedMatVector &frame, ImuVector &imu);

  /*!
  \brief Get DVS+APS+IMU data.
  \param events Event queue to which events will be added
  \param frames Frame queue to which frame will be added
  \param imu Imu data queue to which imu data will be added
  \return True if event queue, frame queue, or imu data queue not empty
  */
  bool getData(Queue &events, StampedMatQueue &frame, ImuQueue &imu);

  /*!
  \brief Retrieve raw event data.
  \param data A vector to which the raw event data will be added.
  \return The number of events added.
  \note Events are encoded as follows:
  Mask for x: 11111111111111100000000000000000 00000000000000000000000000000000
  Mask for y: 00000000000000011111111111111100 00000000000000000000000000000000
  Mask for p: 00000000000000000000000000000010 00000000000000000000000000000000
  Mask for t: 00000000000000000000000000000000 11111111111111111111111111111111
  */
  std::size_t getEventRaw(std::vector<uint64_t> &data);

  /*!
  \brief Retrieve raw event data.
  \param data Buffer to which the raw event data will be written. Updated in place if the buffer is reallocated.
  \param allow_realloc If true, this function manages the size of the buffer, reallocating if needed. The buffer must then come from
  malloc/calloc/realloc (never from new[]) and remains owned by the caller, which is responsible for freeing it. If false, the caller
  must guarantee that the buffer is large enough: no bounds checking is performed.
  \return The number of events written.
  \note Events are encoded as follows:
  Mask for x: 11111111111111100000000000000000 00000000000000000000000000000000
  Mask for y: 00000000000000011111111111111100 00000000000000000000000000000000
  Mask for p: 00000000000000000000000000000010 00000000000000000000000000000000
  Mask for t: 00000000000000000000000000000000 11111111111111111111111111111111
  */
  std::size_t getEventRaw(uint64_t *&data, const bool allow_realloc = true);

  /*!
  \brief Discard data during an interval of time.
  \param usec Time interval in microseconds
  */
  void flush(const double usec) const;

protected:
  /*! \cond INTERNAL */
  template <typename T1, typename T2, typename T3>
  bool getData_([[maybe_unused]] T1 *dvs, [[maybe_unused]] T2 *aps, [[maybe_unused]] T3 *imu);

  template <typename T>
  std::size_t getEventRaw_(T &data, [[maybe_unused]] const bool allow_realloc);

  void interpolate_(cv::Mat &frame) const;

  /*!
  \brief Supress pixels by hardware.
  \param pixels Pixels to suppress
  \return Number of pixels the device took
  \note Whatever is not taken stays for the software filter.
  */
  virtual std::size_t setDvsFilterPixels([[maybe_unused]] const std::vector<cv::Point> &pixels) {
    return 0;
  }

  std::atomic<bool> running_{false};
  bool hasDefectivePixels_{false};
  cv::Mat hotPixels_;
  std::vector<cv::Point> saturatedPixels_;
  caerDeviceHandle deviceHandler_{nullptr};
  uint64_t resetTime_{0};
  cv::Rect_<uint16_t> roi_;
  bool filterRoiInSoftware_{false};
  /*! \endcond */
};

} // namespace ev

#endif // OPENEV_DEVICES_ABSTRACT_CAMERA_HPP
