/*!
\example example-event-image.cpp
This is an example of how to use the EventImage_<T> class.
*/
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <openev/openev.hpp>

inline void show(const ev::EventImage3b &img) {
  ev::EventImage3b resized;
  cv::resize(img, resized, ev::Size(500, 500), 0, 0, cv::INTER_NEAREST);
  cv::imshow("example-event-image", resized);
  cv::waitKey(5);
}

int main(int argc, const char *argv[]) {
  ev::EventImage3b eimage(30, 30);
  eimage.setColors(cv::viz::Color::bluberry(), cv::viz::Color::cherry(), cv::viz::Color::black());

  int row = 0;
  int col = 0;
  int direction = 0;
  int offset = 0;
  double t = 0;

  while(offset < 0.55 * eimage.cols) {
    t += rand() % 10;
    eimage.insert(ev::Event(col, row, t, (rand() % 2) ? ev::POSITIVE : ev::NEGATIVE));
    switch(direction) {
    case 0:
      col++;
      if(col >= eimage.cols - 1 - offset) direction++;
      break;
    case 1:
      row++;
      if(row >= eimage.rows - 1 - offset) direction++;
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
    show(eimage);
  }
  return 0;
}
