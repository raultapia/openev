/*!
\file hdf5-reader.hpp
\brief HDF5 reader.
\author Raul Tapia
*/
#ifndef OPENEV_READERS_HDF5_READER_HPP
#define OPENEV_READERS_HDF5_READER_HPP

#include "openev/readers/abstract-reader.hpp"
#include <H5Cpp.h>
#include <cstddef>
#include <string>
#include <vector>

namespace ev {

/*!
\brief This class extends AbstractReader_ to read event data from HDF5 files.

Events are expected to be stored as four separate 1-D datasets (one per field).
Any numeric storage type is accepted; values are converted on read.

\code{.cpp}
// Default paths match the common /events/{t,x,y,p} layout:
ev::HDF5Reader reader("recording.h5");

// Custom paths:
ev::HDF5Reader reader("recording.h5",
                      "/davis/left/events/timestamp",
                      "/davis/left/events/x",
                      "/davis/left/events/y",
                      "/davis/left/events/polarity");
\endcode
*/
class HDF5Reader : public AbstractReader_ {
public:
  explicit HDF5Reader(const std::string &filename,
                      const std::string &t_path = "/events/t",
                      const std::string &x_path = "/events/x",
                      const std::string &y_path = "/events/y",
                      const std::string &p_path = "/events/p",
                      std::size_t buffer_size = 0,
                      bool use_threading = false);
  ~HDF5Reader() override = default;

  /*! \cond INTERNAL */
  HDF5Reader(const HDF5Reader &) = delete;
  HDF5Reader(HDF5Reader &&) noexcept = delete;
  HDF5Reader &operator=(const HDF5Reader &) = delete;
  HDF5Reader &operator=(HDF5Reader &&) noexcept = delete;
  /*! \endcond */

private:
  H5::H5File file_;
  H5::DataSet t_ds_, x_ds_, y_ds_, p_ds_;
  hsize_t total_{0};
  hsize_t file_pos_{0};

  static constexpr hsize_t kChunk = 4096;
  std::vector<double> t_buf_;
  std::vector<int> x_buf_;
  std::vector<int> y_buf_;
  std::vector<int> p_buf_;
  hsize_t buf_pos_{0};
  hsize_t buf_size_{0};

  bool updateBuffer_() override;
};

} // namespace ev

#endif // OPENEV_READERS_HDF5_READER_HPP
