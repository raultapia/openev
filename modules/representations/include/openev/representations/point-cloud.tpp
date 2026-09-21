#ifndef OPENEV_REPRESENTATIONS_POINT_CLOUD_TPP
#define OPENEV_REPRESENTATIONS_POINT_CLOUD_TPP

#ifndef OPENEV_REPRESENTATIONS_POINT_CLOUD_HPP
#include "openev/representations/point-cloud.hpp"
#endif

namespace ev {

template <typename T, const RepresentationOptions Options, typename E>
void PointCloud_<T, Options, E>::visualize(const int t, const double time_scale /*= 1.0*/, const double axis_size /*= 1.0*/, const double point_size /*= 2.0*/) {
  const std::array<const char *, 2> names{"Negative events", "Positive events"};
  const std::array<typename TypeHelper<T>::Type, 2> values{PointCloud_<T, Options, E>::V_OFF, PointCloud_<T, Options, E>::V_ON};
  const cv::Affine3d scaleTransform(cv::Matx44d(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, time_scale, 0.0, 0.0, 0.0, 0.0, 1.0));

  for(std::size_t polarity = 0; polarity < 2; polarity++) {
    if(points_[polarity].empty()) {
      try {
        window_.removeWidget(names[polarity]);
      } catch(const cv::Exception &) {
      }
      continue;
    }
    cv::viz::WCloud cloud(points_[polarity]);
    cloud.setRenderingProperty(cv::viz::POINT_SIZE, point_size);
    cloud.setColor(TypeHelper<T>::convert(values[polarity]));
    if(time_scale != 1.0) {
      cloud.applyTransform(scaleTransform);
    }
    window_.showWidget(names[polarity], cloud);
  }

  window_.setBackgroundColor(TypeHelper<T>::convert(PointCloud_<T, Options, E>::V_RESET));
  window_.showWidget("Coordinate System", cv::viz::WCoordinateSystem(axis_size));

  if(t) {
    window_.spinOnce(t, true);
  } else {
    window_.spin();
  }
}

template <typename T, const RepresentationOptions Options, typename E>
void PointCloud_<T, Options, E>::clear_() {
  points_[0].clear();
  points_[1].clear();
}

template <typename T, const RepresentationOptions Options, typename E>
void PointCloud_<T, Options, E>::clear_(const cv::Mat &background) {
  points_[0].clear();
  points_[1].clear();

  cv::viz::WImage3D image_widget(background, background.size());
  window_.showWidget("Image Plane", image_widget, cv::Affine3d(cv::Matx33d::eye(), cv::Vec3d(background.cols / 2.0, background.rows / 2.0, 0)));
}

template <typename T, const RepresentationOptions Options, typename E>
bool PointCloud_<T, Options, E>::insert_(const Event_<E> &e) {
  return (points_[e.p].emplace_back(e), true);
}

} // namespace ev

#endif // OPENEV_REPRESENTATIONS_POINT_CLOUD_TPP
