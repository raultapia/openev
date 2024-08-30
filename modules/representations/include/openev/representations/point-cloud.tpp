#ifndef OPENEV_REPRESENTATIONS_POINT_CLOUD_TPP
#define OPENEV_REPRESENTATIONS_POINT_CLOUD_TPP

#ifndef OPENEV_REPRESENTATIONS_POINT_CLOUD_HPP
#include "openev/representations/point-cloud.hpp"
#endif

namespace ev {

#if OE_HAVE_VIZ
template <typename T, const RepresentationOptions Options>
void PointCloud_<T, Options>::visualize(const int t, const double time_scale /*= 1.0*/, const double axis_size /*= 1.0*/, const double point_size /*= 2.0*/) {
  if(points_[static_cast<std::size_t>(ev::POSITIVE)].empty() || points_[static_cast<std::size_t>(ev::NEGATIVE)].empty()) { // FIXME: This should be able to display only positive/negative events
    return;
  }
  std::array<cv::viz::WCloud, 2> cloud{cv::viz::WCloud(points_[static_cast<std::size_t>(ev::POSITIVE)]), cv::viz::WCloud(points_[static_cast<std::size_t>(ev::NEGATIVE)])};
  cv::viz::WCoordinateSystem coord_sys_widget(axis_size);

  cloud[static_cast<std::size_t>(ev::POSITIVE)].setRenderingProperty(cv::viz::POINT_SIZE, point_size);
  cloud[static_cast<std::size_t>(ev::NEGATIVE)].setRenderingProperty(cv::viz::POINT_SIZE, point_size);
  cloud[static_cast<std::size_t>(ev::POSITIVE)].setColor(TypeHelper<T>::convert(PointCloud_<T, Options>::ON));
  cloud[static_cast<std::size_t>(ev::NEGATIVE)].setColor(TypeHelper<T>::convert(PointCloud_<T, Options>::OFF));
  if(time_scale != 1.0) {
    const cv::Affine3d scaleTransform(cv::Matx44d(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, time_scale, 0.0, 0.0, 0.0, 0.0, 1.0));
    cloud[static_cast<std::size_t>(ev::POSITIVE)].applyTransform(scaleTransform);
    cloud[static_cast<std::size_t>(ev::NEGATIVE)].applyTransform(scaleTransform);
  }

  window_.setBackgroundColor(TypeHelper<T>::convert(PointCloud_<T, Options>::RESET));
  window_.showWidget("Positive events", cloud[static_cast<std::size_t>(ev::POSITIVE)]);
  window_.showWidget("Negative events", cloud[static_cast<std::size_t>(ev::NEGATIVE)]);
  window_.showWidget("Coordinate System", coord_sys_widget);
  if(t) {
    window_.spinOnce(t, true);
  } else {
    window_.spin();
  }
}
#endif

template <typename T, const RepresentationOptions Options>
void PointCloud_<T, Options>::clear_() {
  points_[0].clear();
  points_[1].clear();
}

template <typename T, const RepresentationOptions Options>
void PointCloud_<T, Options>::clear_(const cv::Mat &background) {
  points_[0].clear();
  points_[1].clear();

#if OE_HAVE_VIZ
  cv::viz::WImage3D image_widget(background, background.size());
  window_.showWidget("Image Plane", image_widget, cv::Affine3d(cv::Matx33d::eye(), cv::Vec3d(background.cols / 2.0, background.rows / 2.0, 0)));
#endif
}

template <typename T, const RepresentationOptions Options>
bool PointCloud_<T, Options>::insert_(const Event &e) {
  return (points_[e.p].emplace_back(e), true);
}

} // namespace ev

#endif // OPENEV_REPRESENTATIONS_POINT_CLOUD_TPP
