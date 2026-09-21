/*!
\file event-camera-dataset.cpp
\brief Implementation of EventCameraDataset.
\author Raul Tapia
*/
#include "openev/datasets/event-camera-dataset.hpp"
#include "openev/core/imu.hpp"
#include "openev/core/types.hpp"
#include "openev/readers/plain-text-reader.hpp"
#include <filesystem>
#include <fstream>
#include <memory>
#include <utility>

std::string ev::EventCameraDataset::name() const {
  return "Event Camera Dataset";
}

std::string ev::EventCameraDataset::shortname() const {
  return "ECD";
}

std::vector<std::string> ev::EventCameraDataset::sequences() const {
  return {SEQUENCES.begin(), SEQUENCES.end()};
}

std::string ev::EventCameraDataset::url(const std::string &sequence) const {
  return "https://rpg.ifi.uzh.ch/datasets/davis/" + sequence + ".zip";
}

cv::Size ev::EventCameraDataset::sensorSize(const std::string & /*sequence*/) const {
  return {WIDTH, HEIGHT};
}

std::string ev::EventCameraDataset::cameraName(const std::string & /*sequence*/) const {
  return "DAVIS240C";
}

bool ev::EventCameraDataset::isComplete_(const std::string &path) const {
  return std::filesystem::exists(std::filesystem::path(path) / "events.txt");
}

void ev::EventCameraDataset::load_(const std::string &sequence) {
  const std::string directory = path(sequence);

  auto reader = std::make_unique<PlainTextReader>(directory + "/events.txt");
  reader->setTimeTransform(0, SECONDS_TO_MICROSECONDS);

  FrameFileVector frames;
  if(std::ifstream file(directory + "/images.txt"); file.is_open()) {
    TimeType t = 0;
    std::string name;
    while(file >> t >> name) {
      frames.push_back({t * SECONDS_TO_MICROSECONDS, directory + "/" + name});
    }
  }

  ImuVector imu;
  if(std::ifstream file(directory + "/imu.txt"); file.is_open()) {
    TimeType t = 0;
    Imu sample;
    while(file >> t >> sample.linear_acceleration.x >> sample.linear_acceleration.y >> sample.linear_acceleration.z >> sample.angular_velocity.x >> sample.angular_velocity.y >> sample.angular_velocity.z) {
      sample.t = t * SECONDS_TO_MICROSECONDS;
      imu.push_back(sample);
    }
  }

  assign_(std::move(reader), std::move(frames), std::move(imu));
}
