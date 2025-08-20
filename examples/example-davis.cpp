/*!
\example example-davis.cpp
This is an example of how to use the Davis class.
*/
#include "openev/containers/vector.hpp"
#include "openev/core/types.hpp"
#include "openev/devices/abstract-camera.hpp"
#include "openev/devices/davis.hpp"
#include "openev/representations/event-histogram.hpp"
#include <cstdlib>
#include <iostream>
#include <memory>
#include <opencv2/core.hpp>
#include <opencv2/core/cvstd.inl.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/mat.inl.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/saturate.hpp>
#include <opencv2/core/traits.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <string>

static inline void show1(ev::EventHistogram3b &evhist) {
  ev::EventHistogram3b resized;
  cv::resize(evhist.render(), resized, ev::Size(), 2, 2, cv::INTER_NEAREST);
  cv::imshow("example-davis: dvs", resized);
  cv::waitKey(1);
}

static inline void show2(cv::Mat &img) {
  cv::Mat resized;
  cv::resize(img, resized, ev::Size(), 2, 2, cv::INTER_NEAREST);
  cv::imshow("example-davis: aps", resized);
  cv::waitKey(1);
}

int main(int /*argc*/, const char * /*argv*/[]) {
  ev::Davis camera;
  camera.setRoi(cv::Rect(20, 20, 250, 200));
  std::cout << camera.getRoi() << '\n';
  camera.enableDvs(true);
  camera.enableAps(true);
  camera.enableImu(false);
  camera.setDvsTimeInterval(33333U); // 30Hz
  camera.setDvsEventsPerPacket(0);   // No limit

  ev::Vector events;
  ev::StampedMat img;
  ev::Imu imu;
  ev::EventHistogram3b evhist(camera.getSensorSize());

  camera.start();
  while(true) {
    events.clear();
    evhist.clear();

    camera.getData(events, img, imu);
    std::cout << events.size() << ", " << !img.empty() << ", " << !imu.empty() << '\n';

    if(!events.empty()) {
      evhist.insert(events);
      show1(evhist);
    }
    if(!img.empty()) {
      show2(img);
    }
    if(!imu.empty()) {
      std::cout << imu << '\n';
    }
  }

  return 0;
}
