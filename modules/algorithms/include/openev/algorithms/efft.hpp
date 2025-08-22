/*!
\file efft.hpp
\brief eFFT published in IEEE Transactions on Pattern Analysis and Machine Intelligence 2024.
\author Raul Tapia
*/
#ifndef OPENEV_ALGORITHMS_EFFT_HPP
#define OPENEV_ALGORITHMS_EFFT_HPP

#include "efft/include/efft.hpp"
#include <complex>
#include <eigen3/Eigen/Core>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/mat.inl.hpp>

namespace ev {
template <typename T>
class Event_;

template <unsigned int N>
class efft : public eFFT<N> {
public:
  efft() {
    eFFT<N>::initialize();
  }

  template <typename E = int>
  inline bool update(const Event_<E> &e, const bool state) {
    return eFFT<N>::update({static_cast<unsigned int>(e.y), static_cast<unsigned int>(e.x), state});
  }

  template <typename E = int>
  inline bool insert(const Event_<E> &e) {
    return eFFT<N>::update({static_cast<unsigned int>(e.y), static_cast<unsigned int>(e.x), true});
  }

  template <typename E = int>
  inline bool extract(const Event_<E> &e) {
    return eFFT<N>::update({static_cast<unsigned int>(e.y), static_cast<unsigned int>(e.x), false});
  }

  inline cv::Mat mat() {
    cv::Mat cvMat(N, N, CV_32FC2);
    Eigen::Map<Eigen::Matrix<std::complex<float>, N, N, Eigen::RowMajor>>(reinterpret_cast<std::complex<float> *>(cvMat.data)) = eFFT<N>::getFFT();
    return cvMat;
  }
};

} // namespace ev

#endif // OPENEV_ALGORITHMS_EFFT_HPP
