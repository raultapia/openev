#ifndef OPENEV_POINT_CLOUD_TPP
#define OPENEV_POINT_CLOUD_TPP

#ifndef OPENEV_REPRESENTATION_HPP
#include "openev/representation.hpp"
#endif

namespace ev {

template <typename T>
void PointCloud_<T>::visualizeOnce() {
  std::array<cv::viz::WCloud, 2> cloud = {cv::viz::WCloud(points_[ev::POSITIVE]), cv::viz::WCloud(points_[ev::NEGATIVE])};
  cv::viz::WCoordinateSystem coord_sys_widget(frame.empty() ? 50 : std::min(frame.height, frame.width));

  cloud[ev::POSITIVE].setRenderingProperty(cv::viz::POINT_SIZE, 4.0);
  cloud[ev::NEGATIVE].setRenderingProperty(cv::viz::POINT_SIZE, 4.0);
  cloud[ev::POSITIVE].setColor(ColorHelper<T>::convert(this->values_[PointCloud_<T>::value::ON]));
  cloud[ev::NEGATIVE].setColor(ColorHelper<T>::convert(this->values_[PointCloud_<T>::value::OFF]));

  window_.setBackgroundColor(ColorHelper<T>::convert(this->values_[PointCloud_<T>::value::RESET]));
  window_.showWidget("Positive events", cloud[ev::POSITIVE]);
  window_.showWidget("Negative events", cloud[ev::NEGATIVE]);
  window_.showWidget("Coordinate System", coord_sys_widget);
  window_.spinOnce(1, true);
}

template <typename T>
void PointCloud_<T>::visualize() {
  while(!window_.wasStopped()) {
    visualizeOnce();
  }
}

template <typename T>
void PointCloud_<T>::clear_() {
  points_[0].clear();
  points_[1].clear();
}

template <typename T>
bool PointCloud_<T>::insert_(const Event &e) {
  if(frame.empty() || e.inside(ev::Rect({0, 0}, frame))) {
    points_[e.p].push_back(cv::Point3_<BasicDataType<T>>(e.x, e.y, scaleFactor_ * e.t));
    return true;
  } else {
    return false;
  }
}

} // namespace ev

#endif // OPENEV_POINT_CLOUD_TPP
