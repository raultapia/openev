/*!
\example example-point-cloud.cpp
This is an example of how to use the PointCloud_<T> class.
*/
#include <openev/openev.hpp>

int main(int argc, const char *argv[]) {
  ev::PointCloud pointcloud;

  int row = 0;
  int col = 0;
  int direction = 0;
  int offset = 0;
  double t = 0;

  while(offset < 30) {
    t += 0.1;
    pointcloud.insert(ev::Event(col, row, t, (rand() % 2) ? ev::POSITIVE : ev::NEGATIVE));
    switch(direction) {
    case 0:
      col++;
      if(col >= 50 - offset) direction++;
      break;
    case 1:
      row++;
      if(row >= 50 - offset) direction++;
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
  }
  pointcloud.visualize(0, 0.4);
  return 0;
}
