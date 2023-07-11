/*!
\example example-event.cpp
This is an example of how to use the Event_<T> class.
*/
#include <iostream>
#include <openev/openev.hpp>

int main(int argc, const char *argv[]) {
  ev::Event e1(1.23, 20, 30, ev::POSITIVE);
  ev::Event e2(5.67, 40, 30, ev::NEGATIVE);
  ev::Rect rect(0, 0, 35, 35);

  std::cout << "e1: " << e1 << std::endl;
  std::cout << "e2: " << e2 << std::endl;
  std::cout << "Distance: " << e1.spaceDistance(e2) << std::endl;
  std::cout << "Difference: " << e1.timeDifference(e2) << std::endl;
  std::cout << "e1 is inside rect?: " << (rect.contains(e1) ? "yes" : "no") << std::endl;
  std::cout << "e2 is inside rect?: " << (rect.contains(e2) ? "yes" : "no") << std::endl;
  return 0;
}
