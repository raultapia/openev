/*!
\file davis.hpp
\brief Camera device driver for DAVIS cameras.
\author Raul Tapia
*/
#ifndef OPENEV_DEVICES_DAVIS_HPP
#define OPENEV_DEVICES_DAVIS_HPP

#include "openev/devices/abstract-camera.hpp"
#include <ostream>
#include <stdint.h>
#include <string>
#include <vector>

namespace ev {
/*!
\brief Coarse/fine bias value, the encoding used by DAVIS cameras.
*/
struct BiasValue {
  uint8_t coarse;
  uint8_t fine;
  [[nodiscard]] friend std::ostream &operator<<(std::ostream &os, const struct BiasValue &value) {
    os << "Coarse: " << static_cast<int>(value.coarse) << ", Fine: " << static_cast<int>(value.fine);
    return os;
  }
};

/*!
\brief This class extends AbstractCamera to operate with DAVIS event cameras. DAVIS cameras offer events (DVS), framed images (APS), and IMU data.
*/
class Davis : public AbstractCamera {
public:
  constexpr static uint32_t DEFAULT_INTERVAL = 20000;
  constexpr static uint32_t DEFAULT_EXPOSURE = 6500;

  /*! \cond INTERNAL */
  Davis();
  Davis(const Davis &) = delete;
  Davis(Davis &&) noexcept = delete;
  Davis &operator=(const Davis &) = delete;
  Davis &operator=(Davis &&) noexcept = delete;
  /*! \endcond */

  /*!
  \brief Initialize the DAVIS camera.
  This function sets up the DAVIS camera for operation. It must be called before any other.
  */
  void start() override;

  /*!
  \brief DAVIS cameras have an APS sensor.
  \return Always true
  */
  [[nodiscard]] bool hasAps() const override {
    return true;
  }

  /*!
  \brief DAVIS cameras have an IMU.
  \return Always true
  */
  [[nodiscard]] bool hasImu() const override {
    return true;
  }

  /*!
  \brief Get device sensor size.
  \return Sensor size
  */
  [[nodiscard]] cv::Size getSensorSize() const override;

  /*!
  \brief Get device serial number, prefixed with the chip model.
  \return Serial number as CHIP-XXXXXXXX, empty if the camera is not open
  */
  [[nodiscard]] std::string getSerialNumber() const override;

  /*!
  \brief Set current ROI. Events outside the ROI are not considered. Images are cropped according to the ROI.
  \param roi ROI
  \return True if valid ROI
  \note Both filters are applied by the device when the model supports them. Events keep full-sensor coordinates, whereas
  frames are delivered already cropped, so their pixel (0, 0) is roi.tl(). Subtract that offset before overlaying them.
  */
  bool setRoi(const cv::Rect_<uint16_t> &roi) override;

  /*!
  \brief Retrieve the bias value associated with the given name.
  \param name The identifier for the bias value to retrieve.
  \return The bias value corresponding to the given name.
  */
  BiasValue getBias(const uint8_t name) const;

  /*!
  \brief Set the bias value associated with the given name.
  \param name The identifier for the bias value to set.
  \param value The new bias value to set.
  \return True if the bias value was successfully set, false otherwise.
  */
  bool setBias(const uint8_t name, const BiasValue &value);

  /*!
  \brief Convert a bias value into the current it produces. The coarse and fine ranges overlap, so a bias with a lower coarse can produce a higher current than one with a higher
  coarse, and a fine of zero produces no current at all regardless of the coarse.
  \param value The bias value to convert.
  \return Current in picoamperes.
  */
  [[nodiscard]] static uint32_t biasToCurrent(const BiasValue &value);

  /*!
  \brief Enable DVS
  \param state True to enable, false to disable
  */
  void enableDvs(bool state);

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
  \brief Get the APS exposure time currently applied by the device.
  \return Exposure time in microseconds
  */
  [[nodiscard]] uint32_t getApsExposure() const;

  /*!
  \brief Set APS exposure time.
  \param usec Exposure time in microseconds
  */
  void setApsExposure(uint32_t usec);

  /*!
  \brief Let the device choose the APS exposure time on its own, aiming to avoid under and over exposure.
  \param state True to enable, false to disable
  */
  void enableApsAutoExposure(bool state);

  /*!
  \brief Enable IMU
  \param state True to enable, false to disable
  */
  void enableImu(bool state);

private:
  std::size_t setDvsFilterPixels(const std::vector<cv::Point> &pixels) override;
};

} // namespace ev

#endif // OPENEV_DEVICES_DAVIS_HPP
