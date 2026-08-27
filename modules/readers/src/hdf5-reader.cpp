/*!
\file hdf5-reader.cpp
\brief Implementation of hdf5-reader.
\author Raul Tapia
*/
#include "openev/readers/hdf5-reader.hpp"
#include <algorithm>
#include <opencv2/core/utils/logger.hpp>

ev::HDF5Reader::HDF5Reader(const std::string &filename,
                           const std::string &t_path,
                           const std::string &x_path,
                           const std::string &y_path,
                           const std::string &p_path,
                           const std::size_t buffer_size,
                           const bool use_threading)
    : AbstractReader_{buffer_size, use_threading},
      file_{filename, H5F_ACC_RDONLY} {
  try {
    t_ds_ = file_.openDataSet(t_path);
    x_ds_ = file_.openDataSet(x_path);
    y_ds_ = file_.openDataSet(y_path);
    p_ds_ = file_.openDataSet(p_path);

    hsize_t dims[1]{};
    t_ds_.getSpace().getSimpleExtentDims(dims);
    total_ = dims[0];
  } catch(const H5::Exception &e) {
    CV_Error(cv::Error::StsError, "ev::HDF5Reader: " + e.getDetailMsg());
  }
}

bool ev::HDF5Reader::updateBuffer_() {
  if(buf_pos_ >= buf_size_) {
    if(file_pos_ >= total_) {
      return false;
    }

    const hsize_t n = std::min(kChunk, total_ - file_pos_);
    hsize_t start[1] = {file_pos_};
    hsize_t count[1] = {n};
    H5::DataSpace mspace(1, count);

    t_buf_.resize(n);
    x_buf_.resize(n);
    y_buf_.resize(n);
    p_buf_.resize(n);

    auto read = [&](H5::DataSet &ds, void *buf, const H5::PredType &mem_type) {
      H5::DataSpace fspace = ds.getSpace();
      fspace.selectHyperslab(H5S_SELECT_SET, count, start);
      ds.read(buf, mem_type, mspace, fspace);
    };

    read(t_ds_, t_buf_.data(), H5::PredType::NATIVE_DOUBLE);
    read(x_ds_, x_buf_.data(), H5::PredType::NATIVE_INT);
    read(y_ds_, y_buf_.data(), H5::PredType::NATIVE_INT);
    read(p_ds_, p_buf_.data(), H5::PredType::NATIVE_INT);

    file_pos_ += n;
    buf_pos_ = 0;
    buf_size_ = n;
  }

  Event e;
  e.t = static_cast<ev::TimeType>(t_buf_[buf_pos_]);
  e.x = x_buf_[buf_pos_];
  e.y = y_buf_[buf_pos_];
  e.p = p_buf_[buf_pos_] > 0;
  ++buf_pos_;
  buffer_.push(e);
  return true;
}
