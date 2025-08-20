/*!
\example example-time-surface.cpp
This is an example of how to use the TimeSurface_<T> class.
*/
#include "openev/core/types.hpp"
#include "openev/representations/time-surface.hpp"
#include <memory>
#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/mat.inl.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/traits.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/viz/types.hpp>
#include <random>
#include <string>

static inline void show(ev::TimeSurface3b &ts) {
  ev::TimeSurface3b resized;
  cv::resize(ts.render(), resized, ev::Size(500, 500), 0, 0, cv::INTER_NEAREST);
  cv::imshow("example-event-image", resized);
  cv::waitKey(5);
}

int main(int /*argc*/, const char * /*argv*/[]) {
  ev::TimeSurface3b timesurface(30, 30);
  timesurface.setColors(cv::viz::Color::bluberry(), cv::viz::Color::cherry(), cv::viz::Color::black());

  int row = 0;
  int col = 0;
  int direction = 0;
  int offset = 0;
  double t = 0;

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> time_dist(0, 9);
  std::uniform_int_distribution<> polarity_dist(0, 1);

  while(offset < 0.55 * timesurface.cols) {
    t += time_dist(gen);
    timesurface.insert(ev::Event(col, row, t, static_cast<bool>(polarity_dist(gen)) ? ev::POSITIVE : ev::NEGATIVE));
    switch(direction) {
    case 0:
      col++;
      if(col >= timesurface.cols - 1 - offset) {
        direction++;
      }
      break;
    case 1:
      row++;
      if(row >= timesurface.rows - 1 - offset) {
        direction++;
      }
      break;
    case 2:
      col--;
      if(col <= offset) {
        direction++;
        offset++;
      }
      break;
    case 3:
      row--;
      if(row <= offset) {
        direction = 0;
      }
    default:
      break;
    }
    show(timesurface);
  }
  return 0;
}
