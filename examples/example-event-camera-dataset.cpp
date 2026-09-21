/*!
\example example-event-camera-dataset.cpp
This is an example of how to use the EventCameraDataset class.
*/
#include "openev/containers/concurrent_queue.hpp"
#include "openev/containers/queue.hpp"
#include "openev/core/frames.hpp"
#include "openev/datasets/event-camera-dataset.hpp"
#include "openev/readers/player.hpp"
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <opencv2/core/types.hpp>
#include <thread>

int main(int /*argc*/, const char * /*argv*/[]) {
  constexpr std::size_t EVENTS_TO_READ = 200000;
  constexpr std::size_t BATCH = 10000;
  constexpr int STEPS = 4;

  ev::EventCameraDataset dataset;

  std::cout << dataset.name() << " (" << dataset.shortname() << "), " << dataset.sequences().size() << " sequences" << '\n';
  std::cout << "Camera " << dataset.cameraName("slider_close") << ", sensor " << dataset.sensorSize("slider_close") << '\n';
  std::cout << "Kept in " << dataset.path("slider_close") << '\n';
  std::cout << "Downloaded from " << dataset.url("slider_close") << (dataset.isAvailable("slider_close") ? " (already available)" : "") << '\n';

  if(!dataset.open("slider_close")) {
    std::cerr << dataset.error() << '\n';
    return EXIT_FAILURE;
  }
  std::cout << "Open " << dataset.sequence() << ": " << dataset.frames().size() << " frames, " << dataset.imu().size() << " inertial measurements" << '\n';
  std::cout << "Region of interest " << dataset.getRoi() << ", prefetch " << dataset.prefetchCapacity() << '\n';

  dataset.setRoi(cv::Rect(60, 40, 120, 100));
  dataset.prefetch(2 * BATCH);
  std::cout << "Region of interest " << dataset.getRoi() << ", prefetch " << dataset.prefetchCapacity() << '\n';

  std::size_t total = 0;
  bool inside = true;
  while(total < EVENTS_TO_READ && !dataset.events(BATCH).empty()) {
    ev::ConcurrentQueue &events = dataset.events();
    while(!events.empty()) {
      inside = inside && dataset.getRoi().contains(events.front());
      events.pop();
      total++;
    }
  }
  std::cout << "Read " << total << " events, " << (inside ? "all" : "not all") << " inside the region of interest" << '\n';

  if(!dataset.open("slider_depth")) {
    std::cerr << dataset.error() << '\n';
    return EXIT_FAILURE;
  }
  std::cout << "Open " << dataset.sequence() << ": region of interest " << dataset.getRoi() << ", prefetch " << dataset.prefetchCapacity() << '\n';

  ev::Player_ player(dataset);
  player.setSpeed(2.0);
  ev::Queue events;
  ev::StampedMatQueue frames;
  for(int i = 0; i < STEPS; i++) {
    player.next(events, frames);
    std::cout << "Playhead at " << player.playhead() / 1e3 << " ms: " << events.size() << " events and " << frames.size() << " frames so far" << '\n';
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  return EXIT_SUCCESS;
}
