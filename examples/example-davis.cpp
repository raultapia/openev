/*!
\example example-davis.cpp
This is an example of how to use the Davis class.
*/
#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <openev/openev.hpp>

inline void show1(ev::EventHistogram3b &evhist) {
  evhist.render();
  ev::EventHistogram3b resized;
  cv::resize(evhist, resized, ev::Size(), 2, 2, cv::INTER_NEAREST);
  cv::imshow("example-davis: dvs", resized);
  cv::waitKey(1);
}

inline void show2(cv::Mat &img) {
  cv::Mat resized;
  cv::resize(img, resized, ev::Size(), 2, 2, cv::INTER_NEAREST);
  cv::imshow("example-davis: aps", resized);
  cv::waitKey(1);
}

int main(int argc, const char *argv[]) {
  ev::Davis camera(ev::device::DAVIS346);
  camera.setRoi(cv::Rect(20, 20, 250, 200));
  std::cout << camera.getRoi() << std::endl;
  camera.enableDvs(true);
  camera.enableAps(true);
  camera.enableImu(false);
  camera.setDvsTimeInterval(33333.333); // 30Hz
  camera.setDvsEventsPerPacket(0);      // No limit

  ev::EventPacket packet;
  ev::StampedMat img;
  ev::Imu imu;
  ev::EventHistogram3b evhist(camera.getSensorSize());

  camera.start();
  while(1) {
    packet.clear();
    evhist.clear();

    camera.getData(packet, img, imu);
    std::cout << packet.size() << ", " << !img.empty() << ", " << !imu.empty() << std::endl;

    if(!packet.empty()) {
      evhist.insert(packet);
      show1(evhist);
    }
    if(!img.empty()) {
      show2(img);
    }
    if(!imu.empty()) {
      std::cout << imu << std::endl;
    }
  }

  return 0;
}
