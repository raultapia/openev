/*!
\file abstract-representation.hpp
\brief Event abstract representation.
\author Raul Tapia
*/
#ifndef OPENEV_REPRESENTATIONS_ABSTRACT_REPRESENTATION_HPP
#define OPENEV_REPRESENTATIONS_ABSTRACT_REPRESENTATION_HPP

#include "openev/containers/array.hpp"
#include "openev/containers/queue.hpp"
#include "openev/containers/vector.hpp"
#include "openev/core/types.hpp"
#include "openev/options.hpp"
#include <array>
#include <cstddef>
#include <float.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/traits.hpp>
#include <stdint.h>
#include <type_traits>

#if OE_HAVE_VIZ
#include <opencv2/viz/types.hpp>
#endif

namespace ev {

enum RepresentationOptions : uint8_t {
  NONE = 0b00000000,
  IGNORE_POLARITY = 0b00000001,
  ONLY_IF_POSITIVE = 0b00000010,
  ONLY_IF_NEGATIVE = 0b00000100
};
constexpr bool REPRESENTATION_OPTION_CHECK(const uint8_t a, const RepresentationOptions b) {
  return static_cast<bool>(a & static_cast<uint8_t>(b));
}

/*! \cond INTERNAL */
template <typename T, typename = void>
struct DataTypeTrait {
  using type = T;
};

template <typename T>
struct DataTypeTrait<T, std::void_t<typename T::value_type>> {
  using type = typename T::value_type;
};

template <typename T>
class TypeHelper {
public:
  using Type = T;
  using TypeArray = std::array<T, 3>;
  using PrimitiveDataType = typename DataTypeTrait<T>::type;
  using FloatingPointType = typename std::conditional<std::is_same<T, double>::value, double, float>::type;
  using ChannelType = typename cv::Mat_<PrimitiveDataType>;
  static constexpr int NumChannels = cv::DataType<T>::channels;

  static constexpr TypeArray initialize() {
    if constexpr(std::is_floating_point_v<PrimitiveDataType>) {
      return TypeArray{repeat(1.0), repeat(-1.0), repeat(0.0)};
    } else {
      if constexpr(NumChannels == 1) {
        constexpr Type white = 255;
        constexpr Type black = 0;
        constexpr Type gray = 128;
        return TypeArray{white, black, gray};
      } else {
        return TypeArray{Type(255, 0, 0), Type(0, 0, 255), Type(0, 0, 0)};
      }
    }
  }

  static constexpr Type repeat(const double &value) {
    if constexpr(NumChannels == 1) {
      return static_cast<Type>(value);
    } else {
      Type ret;
      for(int i = 0; i < NumChannels; i++) {
        ret[i] = value;
      }
      return ret;
    }
  }

#if OE_HAVE_VIZ
  static constexpr Type convert(const cv::viz::Color &color) {
    if constexpr(NumChannels == 1) {
      return color[0];
    } else {
      Type ret;
      for(int i = 0; i < NumChannels; i++) {
        ret[i] = color[i];
      }
      return ret;
    }
  }

  static cv::viz::Color convert(const Type &noncolor) {
    if constexpr(NumChannels == 1) {
      return cv::viz::Color(noncolor);
    } else {
      cv::viz::Color ret;
      for(int i = 0; i < NumChannels; i++) {
        ret[i] = noncolor[i];
      }
      return ret;
    }
  }
#endif
};
/*! \endcond */

/*!
\brief This is an auxiliary class. This class cannot be instanced.
*/
template <typename T, const RepresentationOptions Options = RepresentationOptions::NONE>
class AbstractRepresentation_ {
public:
  using Type = typename TypeHelper<T>::Type; /*!< Type */

  /*!
  \brief Number of events integrated in the representation.
  \return Number of events
  */
  [[nodiscard]] inline std::size_t count() const { return count_; }

  /*!
  \brief Time difference between the oldest and the newest event integrated in the representation.
  \return Time difference. Returns -1 if time limits are not properly set.
  */
  [[nodiscard]] inline double duration() const {
    if(tLimits_[MIN] == DBL_MAX || tLimits_[MAX] == DBL_MIN) {
      return -1;
    }
    return tLimits_[MAX] - tLimits_[MIN];
  }

  /*!
  \brief Calculate the midpoint time between the oldest and the newest event.
  \return Midpoint time. Returns -1 if time limits are not properly set.
  */
  [[nodiscard]] inline double midTime() const {
    if(tLimits_[MIN] == DBL_MAX || tLimits_[MAX] == DBL_MIN) {
      return -1;
    }
    return 0.5 * (tLimits_[MAX] + tLimits_[MIN]);
  }

  /*!
  \brief Remove all events from the representation.
  \note The way in which the events are removed should be implemented in the derived classes.
  */
  void clear();

  /*!
  \brief Remove all events from the representation and add a background image.
  \note The way in which the events are removed should be implemented in the derived classes.
  */
  void clear(const cv::Mat &background);

  /*!
  \brief Insert one event in the representation.
  \param e Event to insert
  \return True if the event has been inserted
  \note The way in which the event is inserted should be implemented in the derived classes.
  */
  bool insert(const Event &e);

  /*!
  \brief Insert an array of events in the representation.
  \param array Event array to insert
  \return True if all the events have been inserted
  */
  template <std::size_t N>
  bool insert(const Array<N> &array);

  /*!
  \brief Insert a vector of events in the representation.
  \param vector Event vector to insert
  \return True if all the events have been inserted
  */
  bool insert(const Vector &vector);

  /*!
  \brief Insert a queue of events in the representation.
  \param queue Event queue to insert
  \param keep_events_in_queue If true, events are reinserted in the queue
  \return True if all the events have been inserted
  */
  bool insert(Queue &queue, const bool keep_events_in_queue = false);

  /*!
  \brief Set time offset.
  \param e Event
  \warning Offset is set to match the event timestamp, i.e., \f$\tau + t = 0\f$ (where \f$ \tau \f$ is the offset and \f$ t \f$ is the timestamp of the event).
  */
  void setTimeOffset(const Event &e) {
    timeOffset_ = -e.t;
  }

  /*!
  \brief Set values for ON and OFF pixels.
  \param polarity Positive (ON) or negative (OFF)
  \param value Value for ON/OFF pixels
  */
  inline void setValue(const bool polarity, const Type &value) {
    if(polarity) {
      V_ON = value;
    } else {
      V_OFF = value;
    }
  }

  /*!
  \brief Set value for non-activated pixels (background).
  \param value Value for non-activated pixels
  */
  inline void setValue(const Type &value) {
    V_RESET = value;
  }

  /*!
  \brief Set values for ON, OFF, and non-activated pixels.
  \param positive Value for ON pixels
  \param negative Value for OFF pixels
  \param reset Value for non-activated pixels
  */
  inline void setValues(const Type &positive, const Type &negative, const Type &reset) {
    V_ON = positive;
    V_OFF = negative;
    V_RESET = reset;
  }

#if OE_HAVE_VIZ
  /*!
  \brief Set colors for ON and OFF pixels. For more information, please refer <a href="https://docs.opencv.org/master/d4/dba/classcv_1_1viz_1_1Color.html">here</a>.
  \param polarity Positive (ON) or negative (OFF)
  \param color Color for ON/OFF pixels
  \note cv::viz::Color inherits from cv::Scalar_<double>. Color are defined as BGR with values between 0 and 255.
  */
  inline void setColor(const bool polarity, const cv::viz::Color &color) {
    setValue(polarity, TypeHelper<T>::convert(color));
  }

  /*!
  \brief Set colors for non-activated pixels (background). For more information, please refer <a href="https://docs.opencv.org/master/d4/dba/classcv_1_1viz_1_1Color.html">here</a>.
  \param color Color for non-activated pixels
  \note cv::viz::Color inherits from cv::Scalar_<double>. Color are defined as BGR with values between 0 and 255.
  */
  inline void setColor(const cv::viz::Color &color) {
    setValue(TypeHelper<T>::convert(color));
  }

  /*!
  \brief Set colors for ON, OFF, and non-activated pixels. For more information, please refer <a href="https://docs.opencv.org/master/d4/dba/classcv_1_1viz_1_1Color.html">here</a>.
  \param positive Color for ON pixels
  \param negative Color for OFF pixels
  \param reset Color for non-activated pixels
  \note cv::viz::Color inherits from cv::Scalar_<double>. Color are defined as BGR with values between 0 and 255.
  */
  inline void setColors(const cv::viz::Color &positive, const cv::viz::Color &negative, const cv::viz::Color &reset) {
    setValues(TypeHelper<T>::convert(positive), TypeHelper<T>::convert(negative), TypeHelper<T>::convert(reset));
  }
#endif

  /*!
  \brief Get current values for ON and OFF pixels.
  \param polarity Positive (ON) or negative (OFF)
  \return Values for ON/OFF pixels
  */
  [[nodiscard]] inline Type getValue(const bool polarity) const {
    return polarity ? V_ON : V_OFF;
  }

  /*!
  \brief Get current values for non-activated pixels (background).
  \return Values for non-activated pixels
  */
  [[nodiscard]] inline Type getValue() const {
    return V_RESET;
  }

protected:
  /*! \cond INTERNAL */
  enum : uint8_t { MIN,
                   MAX };

  Type V_ON = TypeHelper<T>::initialize()[0];
  Type V_OFF = TypeHelper<T>::initialize()[1];
  Type V_RESET = TypeHelper<T>::initialize()[2];

  double timeOffset_{0};
  std::array<double, 2> tLimits_{DBL_MAX, DBL_MIN};
  std::size_t count_{0};

  virtual void clear_() = 0;
  virtual void clear_(const cv::Mat &background) = 0;
  virtual bool insert_(const Event &e) = 0;
  /*! \endcond */
};

} // namespace ev

/*! \cond INTERNAL */
#include "openev/representations/abstract-representation.tpp"
/*! \endcond */

#endif // OPENEV_REPRESENTATIONS_ABSTRACT_REPRESENTATION_HPP
