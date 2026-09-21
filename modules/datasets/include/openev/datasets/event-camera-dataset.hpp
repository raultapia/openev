/*!
\file event-camera-dataset.hpp
\brief The Event-Camera Dataset.
\author Raul Tapia
*/
#ifndef OPENEV_DATASETS_EVENT_CAMERA_DATASET_HPP
#define OPENEV_DATASETS_EVENT_CAMERA_DATASET_HPP

#include "openev/datasets/abstract-dataset.hpp"
#include <array>
#include <opencv2/core/types.hpp>
#include <string>
#include <vector>

namespace ev {
[[maybe_unused]] constexpr bool USING_EVENT_CAMERA_DATASET_HPP = true;

/*!
\brief This class extends AbstractDataset to download and open the sequences of the Event-Camera Dataset.

\note E. Mueggler, H. Rebecq, G. Gallego, T. Delbruck and D. Scaramuzza, "The Event-Camera Dataset and Simulator: Event-based
Data for Pose Estimation, Visual Odometry, and SLAM", International Journal of Robotics Research, Vol. 36, Issue 2, 2017.
*/
class EventCameraDataset : public AbstractDataset {
public:
  static constexpr int WIDTH = 240;  /*!< Sensor width in pixels */
  static constexpr int HEIGHT = 180; /*!< Sensor height in pixels */
  static constexpr double SECONDS_TO_MICROSECONDS = 1e6;

  /*!
  \brief Sequences published by the dataset.
  */
  static constexpr std::array<const char *, 27> SEQUENCES{
      "shapes_rotation", "shapes_translation", "shapes_6dof",
      "poster_rotation", "poster_translation", "poster_6dof",
      "boxes_rotation", "boxes_translation", "boxes_6dof",
      "hdr_poster", "hdr_boxes",
      "outdoors_walking", "outdoors_running",
      "dynamic_rotation", "dynamic_translation", "dynamic_6dof",
      "calibration",
      "office_zigzag", "office_spiral",
      "urban",
      "slider_close", "slider_far", "slider_hdr_close", "slider_hdr_far", "slider_depth",
      "simulation_3planes", "simulation_3walls",
  };

  [[nodiscard]] std::string name() const override;
  [[nodiscard]] std::string shortname() const override;
  [[nodiscard]] std::vector<std::string> sequences() const override;
  [[nodiscard]] std::string url(const std::string &sequence) const override;
  [[nodiscard]] cv::Size sensorSize(const std::string &sequence) const override;
  [[nodiscard]] std::string cameraName(const std::string &sequence) const override;

protected:
  /*! \cond INTERNAL */
  [[nodiscard]] bool isComplete_(const std::string &path) const override;
  void load_(const std::string &sequence) override;
  /*! \endcond */
};

} // namespace ev

#endif // OPENEV_DATASETS_EVENT_CAMERA_DATASET_HPP
