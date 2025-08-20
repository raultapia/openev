/*!
\example example-event.cpp
This is an example of how to use the Event_<T> class.
*/
#include "openev/core/types.hpp"
#include <iostream>

int main(int /*argc*/, const char * /*argv*/[]) {
  const ev::Event e1(20, 30, 1.23, ev::POSITIVE);
  const ev::Event e2(40, 30, 5.67, ev::NEGATIVE);
  const ev::Rect rect(0, 0, 35, 35);

  std::cout << "e1: " << e1 << '\n';
  std::cout << "e2: " << e2 << '\n';
  std::cout << "Distance between e1 and e2 = " << e1.distance(e2) << '\n';
  std::cout << "e1 is inside rect?: " << (rect.contains(e1) ? "yes" : "no") << '\n';
  std::cout << "e2 is inside rect?: " << (rect.contains(e2) ? "yes" : "no") << '\n';
  return 0;
}
