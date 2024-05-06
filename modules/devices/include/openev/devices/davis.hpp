/*!
\file davis.hpp
\brief Camera device driver for DAVIS cameras.
\author Raul Tapia
*/
#ifndef OPENEV_DEVICES_DAVIS_HPP
#define OPENEV_DEVICES_DAVIS_HPP

#include "openev/containers/queue.hpp"
#include "openev/containers/vector.hpp"
#include "openev/devices/abstract-camera.hpp"
#include <stdint.h>

namespace ev {
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

  BiasValue getBias(const uint8_t name) const;

  bool setBias(const uint8_t name, const BiasValue &value);

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

} // namespace ev

#endif // OPENEV_DEVICES_DAVIS_HPP
