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

/*!
\brief This class extends AbstractCamera_ to operate with DAVIS event cameras. DAVIS cameras offer events (DVS), framed images (APS), and IMU data.
*/
class Davis_ : public AbstractCamera_ {
public:
  constexpr static uint32_t DEFAULT_INTERVAL = 20000;
  constexpr static uint32_t DEFAULT_EXPOSURE = 6500;

  /*! \cond INTERNAL */
  Davis_();
  ~Davis_() override;
  Davis_(const Davis_ &) = delete;
  Davis_(Davis_ &&) noexcept = delete;
  Davis_ &operator=(const Davis_ &) = delete;
  Davis_ &operator=(Davis_ &&) noexcept = delete;
  /*! \endcond */

  inline BiasValue getBias(const uint8_t name) const {
    return AbstractCamera_::getBias<DAVIS_CONFIG_BIAS>(name);
  }

  inline bool setBias(const uint8_t name, const BiasValue &value) {
    return AbstractCamera_::setBias<DAVIS_CONFIG_BIAS>(name, value);
  }

  /*!
  \brief Enable DVS
  \param state True to enable, false to disable
  */
  void enableDvs(bool state);

  /*!
  \brief Set DVS maximum time interval between subsequent transmissions.
  \warning Set to zero to disable.
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
  bool getData(Vector &events) override;

  /*!
  \brief Get DVS data.
  \param events Event queue to which events will be added.
  \return True if queue not empty
  */
  bool getData(Queue &events) override;

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

private:
  bool dvsEnabled_{true};
  bool apsEnabled_{true};
  bool imuEnabled_{false};
  template <typename T1, typename T2, typename T3>
  void getData_([[maybe_unused]] T1 *dvs, [[maybe_unused]] T2 *aps, [[maybe_unused]] T3 *imu);
};

class Davis346 final : public Davis_ {
public:
  struct Bias {
    constexpr static uint8_t REFR = DAVIS346_CONFIG_BIAS_REFRBP; /*!< Refractory period bias */
    constexpr static uint8_t PR = DAVIS346_CONFIG_BIAS_PRBP;     /*!< Photoreceptor bias */
    constexpr static uint8_t PRSF = DAVIS346_CONFIG_BIAS_PRSFBP; /*!< Source-follower bias */
    constexpr static uint8_t DIFF = DAVIS346_CONFIG_BIAS_DIFFBN; /*!< Differencing amp bias */
    constexpr static uint8_t ON = DAVIS346_CONFIG_BIAS_ONBN;     /*!< ON comparator bias */
    constexpr static uint8_t OFF = DAVIS346_CONFIG_BIAS_OFFBN;   /*!< OFF comparator bias */
    constexpr static int DEFAULT_COARSE_VALUE_1 = 2;
    constexpr static int DEFAULT_FINE_VALUE_1 = 116;
    constexpr static int DEFAULT_COARSE_VALUE_2 = 1;
    constexpr static int DEFAULT_FINE_VALUE_2 = 33;
  };

  Davis346() {
    struct caer_bias_coarsefine b;
    b.coarseValue = Bias::DEFAULT_COARSE_VALUE_1;
    b.fineValue = Bias::DEFAULT_FINE_VALUE_1;
    b.enabled = true;
    b.sexN = false;
    b.typeNormal = true;
    b.currentLevelNormal = true;
    caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_BIAS, DAVIS346_CONFIG_BIAS_PRBP, caerBiasCoarseFineGenerate(b));
    b.coarseValue = Bias::DEFAULT_COARSE_VALUE_2;
    b.fineValue = Bias::DEFAULT_FINE_VALUE_2;
    b.enabled = true;
    b.sexN = false;
    b.typeNormal = true;
    b.currentLevelNormal = true;
    caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_BIAS, DAVIS346_CONFIG_BIAS_PRSFBP, caerBiasCoarseFineGenerate(b));
    ev::logger::info("DAVIS346 device configured.");
  }
};

} // namespace ev

#endif // OPENEV_CAMERA_HPP
