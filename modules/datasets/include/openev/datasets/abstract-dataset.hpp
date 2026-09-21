/*!
\file abstract-dataset.hpp
\brief Abstract base class for the datasets of event-based vision.
\author Raul Tapia
*/
#ifndef OPENEV_DATASETS_ABSTRACT_DATASET_HPP
#define OPENEV_DATASETS_ABSTRACT_DATASET_HPP

#include "openev/containers/concurrent_queue.hpp"
#include "openev/core/frames.hpp"
#include "openev/core/imu.hpp"
#include "openev/readers/abstract-reader.hpp"
#include <cstddef>
#include <functional>
#include <memory>
#include <opencv2/core/types.hpp>
#include <string>
#include <vector>

namespace ev {
[[maybe_unused]] constexpr bool USING_ABSTRACT_DATASET_HPP = true;

/*!
\brief This is an auxiliary class. This class cannot be instanced.

A dataset knows which sequences it publishes, where each one is downloaded from, and where they are kept in this machine.

\note Events are read on demand: events(n) reads n events from the sequence, adds them to a queue and returns it. With prefetch(n), a background thread keeps n events in the queue instead.
\note Frames are listed by frames() as image files with their timestamps, sorted by time.
\note Inertial measurements are all loaded when the sequence is opened, and listed by imu() sorted by time.
*/
class AbstractDataset {
public:
  /*!
  \brief Function called while a sequence is being downloaded.
  */
  using Progress = std::function<void(double, double)>;

  AbstractDataset() = default;
  virtual ~AbstractDataset() = default;
  AbstractDataset(const AbstractDataset &) = delete;
  AbstractDataset(AbstractDataset &&) noexcept = default;
  AbstractDataset &operator=(const AbstractDataset &) = delete;
  AbstractDataset &operator=(AbstractDataset &&) noexcept = default;

  /*!
  \brief Name of the dataset.
  \return Name
  */
  [[nodiscard]] virtual std::string name() const = 0;

  /*!
  \brief Short name of the dataset, also used as directory name.
  \return Short name
  */
  [[nodiscard]] virtual std::string shortname() const = 0;

  /*!
  \brief Sequences published by the dataset.
  \return Sequence names
  */
  [[nodiscard]] virtual std::vector<std::string> sequences() const = 0;

  /*!
  \brief Address of the zip file of a sequence.
  \param sequence Sequence name
  \return URL
  */
  [[nodiscard]] virtual std::string url(const std::string &sequence) const = 0;

  /*!
  \brief Sensor size of the camera that recorded a sequence.
  \param sequence Sequence name
  \return Sensor size in pixels
  */
  [[nodiscard]] virtual cv::Size sensorSize(const std::string &sequence) const = 0;

  /*!
  \brief Name of the camera that recorded a sequence.
  \param sequence Sequence name
  \return Camera name
  */
  [[nodiscard]] virtual std::string cameraName(const std::string &sequence) const = 0;

  /*!
  \brief Directory where downloaded datasets are kept by default.
  \return $XDG_CACHE_HOME/openev/datasets, or ~/.cache/openev/datasets if XDG_CACHE_HOME is not set
  */
  [[nodiscard]] static std::string cacheDirectory();

  /*!
  \brief Set the directory where the sequences of this dataset are kept.
  \param directory Directory; if empty, shortname() inside cacheDirectory()
  */
  void setDirectory(const std::string &directory);

  /*!
  \brief Directory where the sequences of this dataset are kept.
  \return Directory
  */
  [[nodiscard]] std::string directory() const;

  /*!
  \brief Directory of a sequence.
  \param sequence Sequence name
  \return Directory, whether the sequence has been downloaded or not
  */
  [[nodiscard]] std::string path(const std::string &sequence) const;

  /*!
  \brief Check if a sequence has been downloaded.
  \param sequence Sequence name
  \return True if the sequence is ready to be read
  */
  [[nodiscard]] bool isAvailable(const std::string &sequence) const;

  /*!
  \brief Download a sequence and extract it in path(). Nothing is done if the sequence is already available.
  \param sequence Sequence name
  \param progress Function called while downloading
  \return True if the sequence is available; if false, see error()
  */
  bool download(const std::string &sequence, const Progress &progress = nullptr);

  /*!
  \brief Open a sequence, downloading it first if it is not available.
  \param sequence Sequence name
  \param progress Function called while downloading
  \return True if the sequence is open; if false, see error()
  */
  bool open(const std::string &sequence, const Progress &progress = nullptr);

  /*!
  \brief Close the sequence.
  */
  void close();

  /*!
  \brief Check if a sequence is open.
  \return True if open
  */
  [[nodiscard]] inline bool isOpen() const {
    return reader_ != nullptr;
  }

  /*!
  \brief Name of the sequence that is open.
  \return Sequence name, empty if none
  */
  [[nodiscard]] inline const std::string &sequence() const {
    return sequence_;
  }

  /*!
  \brief Read a given number of events, add them to the queue, and return it, see AbstractReader_::events().
  \param n Number of events to add to the queue
  \return Reference to the queue
  */
  [[nodiscard]] inline ConcurrentQueue &events(const std::size_t n = 0) {
    return reader_->events(n);
  }

  /*!
  \brief Keep the queue at a given number of events from a background thread, see AbstractReader_::prefetch().
  \param n Number of events to keep in the queue; zero stops the thread
  */
  inline void prefetch(const std::size_t n) {
    reader_->prefetch(n);
  }

  /*!
  \brief Number of events prefetch() keeps in the queue, see AbstractReader_::prefetchCapacity().
  \return Number of events, zero if there is no thread
  */
  [[nodiscard]] inline std::size_t prefetchCapacity() const {
    return reader_->prefetchCapacity();
  }

  /*!
  \brief Deliver only the events inside a region of interest, as the cameras of the devices module do. The events keep their coordinates.
  \param roi Region of interest in pixels; an empty region delivers every event
  \note Frames and inertial measurements are not affected.
  */
  inline void setRoi(const cv::Rect &roi) {
    reader_->setRoi(roi);
  }

  /*!
  \brief Get the region of interest.
  \return Region of interest in pixels; the whole sensor of the sequence that is open if none was set
  */
  [[nodiscard]] inline cv::Rect getRoi() const {
    const cv::Rect roi = reader_->getRoi();
    return roi.empty() ? cv::Rect(cv::Point(0, 0), sensorSize(sequence_)) : roi;
  }

  /*!
  \brief Go back to the beginning of the sequence, see AbstractReader_::reset().
  */
  inline void reset() {
    reader_->reset();
  }

  /*!
  \brief Frames of the sequence, empty if it has none.
  \return Frames sorted by timestamp, in microseconds, with the absolute path of each image file
  */
  [[nodiscard]] inline const FrameFileVector &frames() const {
    return frames_;
  }

  /*!
  \brief Inertial measurements of the sequence, empty if it has none.
  \return Measurements sorted by timestamp
  */
  [[nodiscard]] inline const ImuVector &imu() const {
    return imu_;
  }

  /*!
  \brief Reason why the last download() failed.
  \return Message, empty if it did not fail
  */
  [[nodiscard]] inline const std::string &error() const {
    return error_;
  }

protected:
  /*! \cond INTERNAL */
  [[nodiscard]] virtual bool isComplete_(const std::string &path) const = 0;
  virtual void load_(const std::string &sequence) = 0;
  void assign_(std::unique_ptr<AbstractReader_> reader, FrameFileVector frames, ImuVector imu);
  std::string error_;
  /*! \endcond */

private:
  std::string directory_;
  std::string sequence_;
  std::unique_ptr<AbstractReader_> reader_;
  FrameFileVector frames_;
  ImuVector imu_;
};

} // namespace ev

#endif // OPENEV_DATASETS_ABSTRACT_DATASET_HPP
