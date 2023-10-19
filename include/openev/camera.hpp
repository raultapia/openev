/*!
\file camera.hpp
\brief Camera device driver. This module uses libcaer library to configure devices and get data. For more information, please refer <a href="https://gitlab.com/inivation/dv/libcaer">here</a>.
\author Raul Tapia
*/
#ifndef OPENEV_CAMERA_HPP
#define OPENEV_CAMERA_HPP

#include "openev/containers.hpp"
#include "openev/types.hpp"
#include <atomic>
#include <chrono>
#include <libcaercpp/devices/davis.hpp>

namespace ev {

constexpr int XYZ_ALIGNEMENT = 32;
constexpr int IMU_ALIGNEMENT = 128;
constexpr double EARTH_GRAVITY = 9.80665;
constexpr double DEG2RAD = M_PI / 180.0;
constexpr double SCALE_16B_8B = 1.0 / 256.0;
constexpr uint32_t DEFAULT_INTERVAL = 20000;
constexpr uint32_t DEFAULT_EXPOSURE = 6500;

using bias = struct caer_bias_coarsefine;
constexpr int DAVIS346_COARSE_VALUE_1 = 2;
constexpr int DAVIS346_FINE_VALUE_1 = 116;
constexpr int DAVIS346_COARSE_VALUE_2 = 1;
constexpr int DAVIS346_FINE_VALUE_2 = 33;
constexpr bias GET_DAVIS_346_BIAS_1() {
  struct caer_bias_coarsefine ret {};
  ret.coarseValue = DAVIS346_COARSE_VALUE_1;
  ret.fineValue = DAVIS346_FINE_VALUE_1;
  ret.enabled = true;
  ret.sexN = false;
  ret.typeNormal = true;
  ret.currentLevelNormal = true;
  return ret;
}
constexpr bias GET_DAVIS_346_BIAS_2() {
  struct caer_bias_coarsefine ret {};
  ret.coarseValue = DAVIS346_COARSE_VALUE_2;
  ret.fineValue = DAVIS346_FINE_VALUE_2;
  ret.enabled = true;
  ret.sexN = false;
  ret.typeNormal = true;
  ret.currentLevelNormal = true;
  return ret;
}

/*!
\brief Event camera models implemented
*/
enum class device {
  NONE,
  DAVIS346
};

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
} __attribute__((aligned(XYZ_ALIGNEMENT)));

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
} __attribute__((aligned(IMU_ALIGNEMENT)));
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
  virtual bool getData(EventVector &events) = 0;

  /*!
  \brief Get data.
  \param events Event queue to which events will be added.
  \return True if queue not empty
  */
  virtual bool getData(EventQueue &events) = 0;

protected:
  /*! \cond INTERNAL */
  caerDeviceHandle deviceHandler_{nullptr};
  double timeOffset_{0};
  cv::Rect roi_;
  /*! \endcond */
};

/*!
\brief This class extends AbstractCamera_ to operate with DAVIS event cameras. DAVIS cameras offer events (DVS), framed images (APS), and IMU data.
*/
class Davis : public AbstractCamera_ {
public:
  /*!
  Contructor using device model.
  \note Currently, the following devices are implemented: DAVIS346.
  \param mode Device model
  */
  explicit Davis(device model);

  /*! \cond INTERNAL */
  ~Davis() override;
  Davis(const Davis &) = delete;
  Davis(Davis &&) noexcept = delete;
  Davis &operator=(const Davis &) = delete;
  Davis &operator=(Davis &&) noexcept = delete;
  /*! \endcond */

  /*!
  \brief Enable DVS
  \param state True to enable, false to disable
  */
  void enableDvs(bool state);

  /*!
  \brief Set DVS maximum time interval between subsequent transmissions.
  \warning It must be at least 1.
  \param usec Maximum time interval in microseconds
  */
  void setDvsTimeInterval(uint32_t usec);

  /*!
  \brief Set DVS maximum number of events per transmission.
  \warning Set to zero to disable.
  \param n Maximum number of events
  */
  void setDvsEventsPerPacket(uint32_t n);

  /*!
  \brief Enable APS
  \param state True to enable, false to disable
  */
  void enableAps(bool state);

  /*!
  \brief Set APS maximum time interval between subsequent transmissions.
  \param usec Maximum time interval in microseconds
  */
  void setApsTimeInterval(uint32_t usec);

  /*!
  \brief Set APS exposure time.
  \param usec Exposure time in microseconds
  */
  void setExposure(uint32_t exposure);

  /*!
  \brief Enable IMU
  \param state True to enable, false to disable
  */
  void enableImu(bool state);

  /*!
  \brief Get DVS data.
  \param events Event vector to which events will be added
  \return True if vector not empty
  */
  bool getData(EventVector &events) override;

  /*!
  \brief Get DVS data.
  \param events Event queue to which events will be added.
  \return True if queue not empty
  */
  bool getData(EventQueue &events) override;

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
  bool getData(EventVector &events, StampedMat &frame);

  /*!
  \brief Get DVS+APS data.
  \param events Event vector to which events will be added
  \param frames Frame vector to which frame will be added
  \return True if event vector or frame vector not empty
  */
  bool getData(EventVector &events, StampedMatVector &frames);

  /*!
  \brief Get DVS+APS data.
  \param events Event queue to which events will be added
  \param frames Frame queue to which frame will be added
  \return True if event queue or frame queue not empty
  */
  bool getData(EventQueue &events, StampedMatQueue &frames);

  /*!
  \brief Get DVS+IMU data.
  \param events Event vector to which events will be added
  \param imu Imu data destination
  \return True if event vector or imu data not empty
  */
  bool getData(EventVector &events, Imu &imu);

  /*!
  \brief Get DVS+IMU data.
  \param events Event vector to which events will be added
  \param imu Imu data vector to which imu data will be added
  \return True if event vector or imu data vector not empty
  */
  bool getData(EventVector &events, ImuVector &imu);

  /*!
  \brief Get DVS+IMU data.
  \param events Event queue to which events will be added
  \param imu Imu data queue to which imu data will be added
  \return True if event queue or imu data queue not empty
  */
  bool getData(EventQueue &events, ImuQueue &imu);

  /*!
  \brief Get DVS+APS+IMU data.
  \param events Event vector to which events will be added
  \param frame Frame destination
  \param imu Imu data destination
  \return True if event vector, frame, or imu data not empty
  */
  bool getData(EventVector &events, StampedMat &frame, Imu &imu);

  /*!
  \brief Get DVS+APS+IMU data.
  \param events Event vector to which events will be added
  \param frames Frame vector to which frame will be added
  \param imu Imu data vector to which imu data will be added
  \return True if event vector, frame vector, or imu data vector not empty
  */
  bool getData(EventVector &events, StampedMatVector &frame, ImuVector &imu);

  /*!
  \brief Get DVS+APS+IMU data.
  \param events Event queue to which events will be added
  \param frames Frame queue to which frame will be added
  \param imu Imu data queue to which imu data will be added
  \return True if event queue, frame queue, or imu data queue not empty
  */
  bool getData(EventQueue &events, StampedMatQueue &frame, ImuQueue &imu);

private:
  template <typename T1, typename T2, typename T3>
  void getData_(T1 *dvs, T2 *aps, T3 *imu);
};

} // namespace ev

#endif // OPENEV_CAMERA_HPP
