/*!
\file datasets.hpp
\brief Include all the OpenEV datasets module.
\author Raul Tapia
*/
#ifndef OPENEV_DATASETS_HPP
#define OPENEV_DATASETS_HPP

#include "openev/datasets/abstract-dataset.hpp"
#include "openev/datasets/event-camera-dataset.hpp"

namespace {
inline void workaroundDatasets() {
  (void)ev::USING_ABSTRACT_DATASET_HPP;
  (void)ev::USING_EVENT_CAMERA_DATASET_HPP;
}
} // namespace

#endif // OPENEV_DATASETS_HPP
