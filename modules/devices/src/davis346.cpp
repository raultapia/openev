/*!
\file davis346.cpp
\brief Implementation of davis346.
\author Raul Tapia
*/
#include "openev/devices/davis346.hpp"
#include "libcaer/devices/davis.h"
#include "libcaer/devices/device.h"
#include "openev/utils/logger.hpp"
#include <stdint.h>

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

ev::Davis346::Davis346() {
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
