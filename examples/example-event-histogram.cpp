/*!
\example example-event-histogram.cpp
This is an example of how to use the EventHistogram_<T> class.
*/
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <openev/openev.hpp>

inline void show(ev::EventHistogram3b &h) {
  ev::EventHistogram3b resized;
  cv::resize(h.render(), resized, ev::Size(500, 500), 0, 0, cv::INTER_NEAREST);
  cv::imshow("example-event-image", resized);
  cv::waitKey(5);
}

int main(int argc, const char *argv[]) {
  ev::EventHistogram3b histogram(30, 30);
  histogram.setColors(cv::viz::Color::bluberry(), cv::viz::Color::cherry(), cv::viz::Color::black());

  for(int k = 1; k < 5; k++) {
    int row = 0;
    int col = 0;
    int direction = 0;
    int offset = 0;
    double t = 0;

    while(offset < 0.1 * k * histogram.cols) {
      t += rand() % 10;
      histogram.insert(ev::Event(col, row, t, ev::POSITIVE));
      switch(direction) {
      case 0:
        col++;
        if(col >= histogram.cols - 1 - offset) direction++;
        break;
      case 1:
        row++;
        if(row >= histogram.rows - 1 - offset) direction++;
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
        if(row <= offset) direction = 0;
      }
      show(histogram);
    }
  }
  return 0;
}
