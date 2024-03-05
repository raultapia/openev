/*!
\file abstract-camera.hpp
\brief Abstract camera device driver.
\author Raul Tapia
*/
#ifndef OPENEV_DEVICES_ABSTRACT_CAMERA_HPP
#define OPENEV_DEVICES_ABSTRACT_CAMERA_HPP

#include "openev/containers.hpp"
#include <atomic>
#include <libcaercpp/devices/davis.hpp>

namespace ev {

namespace unit {
template <typename T>
constexpr double us(const T x) { return static_cast<double>(x) * 1e-6; }
} // namespace unit

constexpr double EARTH_GRAVITY = 9.80665;
constexpr double DEG2RAD = M_PI / 180.0;
constexpr double SCALE_16B_8B = 1.0 / 256.0;

struct BiasValue {
  uint8_t coarse;
  uint8_t fine;
  [[nodiscard]] friend std::ostream &operator<<(std::ostream &os, const struct BiasValue &value) {
    os << "Coarse: " << static_cast<int>(value.coarse) << ", Fine: " << static_cast<int>(value.fine);
    return os;
  }
} __attribute__((aligned(2)));

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
  double t;
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
} __attribute__((aligned(32)));

/*!
\brief This struct is used to store IMU data from a DAVIS event camera.

The following aliases are defined for convenience:
\code{.cpp}
using ImuVector = std::vector<Imu>;
using ImuQueue = std::queue<Imu>;
\endcode
*/
struct Imu {
  double t{0};
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
} __attribute__((aligned(128)));
using ImuVector = std::vector<Imu>;
using ImuQueue = std::queue<Imu>;

/*!
\brief This is an auxiliary class. This class cannot be instanced.
*/
class AbstractCamera_ {
public:
  /*! \cond INTERNAL */
  AbstractCamera_() = default;
  virtual ~AbstractCamera_() = default;
  AbstractCamera_(const AbstractCamera_ &) = delete;
  AbstractCamera_(AbstractCamera_ &&) noexcept = delete;
  AbstractCamera_ &operator=(const AbstractCamera_ &) = delete;
  AbstractCamera_ &operator=(AbstractCamera_ &&) noexcept = delete;
  /*! \endcond */

  /*!
  \brief Set offset to add after receiving events.
  \param offset Time offset
  */
  inline void setTimeOffset(const double offset) {
    timeOffset_ = offset;
  }

  /*!
  \brief Start reading events.
  */
  void start();

  /*!
  \brief Stop reading events.
  */
  void stop();

  /*!
  \brief Get device sensor size.
  \return Sensor size
  */
  [[nodiscard]] cv::Size getSensorSize() const;

  /*!
  \brief Get current ROI.
  \return ROI
  */
  [[nodiscard]] cv::Rect getRoi() const;

  /*!
  \brief Set current ROI. Events outside the ROI are not considered. Images are cropped according to the ROI.
  \param roi ROI
  \return True if valid ROI
  */
  bool setRoi(const cv::Rect &roi);

  template <int8_t ABSTRACT_CONFIG_BIAS>
  BiasValue getBias(const uint8_t name) const {
    uint32_t param{0};
    caerDeviceConfigGet(deviceHandler_, ABSTRACT_CONFIG_BIAS, name, &param);
    return {caerBiasCoarseFineParse(param).coarseValue, caerBiasCoarseFineParse(param).fineValue};
  }

  template <int8_t ABSTRACT_CONFIG_BIAS>
  bool setBias(const uint8_t name, const BiasValue &value) {
    uint32_t param{0};
    caerDeviceConfigGet(deviceHandler_, ABSTRACT_CONFIG_BIAS, name, &param);
    struct caer_bias_coarsefine cf = caerBiasCoarseFineParse(param);
    cf.coarseValue = value.coarse;
    cf.fineValue = value.fine;
    return caerDeviceConfigSet(deviceHandler_, ABSTRACT_CONFIG_BIAS, name, caerBiasCoarseFineGenerate(cf));
  }

  /*!
  \brief Discard data during an interval of time.
  \param msec Time interval in milliseconds
  */
  void flush(double msec) const;

  /*!
  \brief Get data.
  \param events Event vector to which events will be added
  \return True if vector not empty
  */
  virtual bool getData(Vector &events) = 0;

  /*!
  \brief Get data.
  \param events Event queue to which events will be added.
  \return True if queue not empty
  */
  virtual bool getData(Queue &events) = 0;

protected:
  /*! \cond INTERNAL */
  std::atomic<bool> running_{false};
  caerDeviceHandle deviceHandler_{nullptr};
  double timeOffset_{0};
  cv::Rect roi_;
  /*! \endcond */
};

} // namespace ev

#endif // OPENEV_DEVICES_ABSTRACT_CAMERA_HPP
