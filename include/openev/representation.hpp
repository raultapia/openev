/*!
\file representation.hpp
\brief Event representations.
\author Raul Tapia
*/
#ifndef OPENEV_REPRESENTATION_HPP
#define OPENEV_REPRESENTATION_HPP

#include "openev/types.hpp"
#include <opencv2/viz/types.hpp>
#include <utility>

namespace ev {

/*!
\brief This is an auxiliary class. This class cannot be instanced.
*/
template <typename T>
class AbstractRepresentation_ : public cv::Mat_<T> {
  using cv::Mat_<T>::Mat_;

public:
  /*! \cond INTERNAL */
  virtual ~AbstractRepresentation_() = default;
  AbstractRepresentation_(const AbstractRepresentation_<T> &) = default;
  AbstractRepresentation_(AbstractRepresentation_<T> &&) noexcept = default;
  AbstractRepresentation_<T> &operator=(const AbstractRepresentation_<T> &) = default;
  AbstractRepresentation_<T> &operator=(AbstractRepresentation_<T> &&) noexcept = default;
  /*! \endcond */

  /*!
  \brief Number of events integrated in the representation.
  \return Number of events
  */
  [[nodiscard]] inline std::size_t count() const { return count_; }

  /*!
  \brief Time difference between the oldest and the newest event integrated in the representation.
  \return Time difference
  */
  [[nodiscard]] inline double duration() const {
    if(tLimits_[MIN] == DBL_MAX || tLimits_[MAX] == DBL_MIN) {
      return 0;
    }
    return tLimits_[MAX] - tLimits_[MIN];
  }

  /*!
  \brief Remove all events from the representation.
  \note The way in which the events are removed should be implemented in the derived classes.
  */
  void clear();

  /*!
  \brief Insert one event in the representation.
  \param e Event to insert
  \return True if the event has been inserted
  \note The way in which the event is inserted should be implemented in the derived classes.
  */
  bool insert(const Event &e);

  /*!
  \brief Insert a packet of events in the representation.
  \param packet Event packet to insert
  \return True if all the events have been inserted
  */
  bool insert(const EventPacket &packet);

  /*!
  \brief Insert a buffer of events in the representation.
  \param buffer Event buffer to insert
  \return True if all the events have been inserted
  */
  bool insert(EventBuffer &buffer);

  /*!
  \brief Set time offset.
  \param e Event
  \warning Offset is set to match the event timestamp, i.e., \f$\tau + t = 0\f$ (where \f$ \tau \f$ is the offset and \f$ t \f$ is the timestamp of the event).
  */
  void setTimeOffset(const Event &e) {
    timeOffset_ = -e.t;
  }

protected:
  /*! \cond INTERNAL */
  virtual void clear_() = 0;
  virtual void insert_(const Event &e) = 0;
  double timeOffset_{0};
  /*! \endcond */

private:
  enum value { MIN,
               MAX };
  std::array<double, 2> tLimits_{DBL_MAX, DBL_MIN};
  std::size_t count_{0};
};

/*!
\private
*/
template <typename T>
class ColorHelper {
public:
  using Value = std::conditional_t<cv::DataType<T>::channels == 1, double, cv::Vec<double, cv::DataType<T>::channels>>;
  using ValueArray = std::array<Value, 3>;

  static constexpr ValueArray initialize() {
    if constexpr(cv::DataType<T>::channels == 1) {
      return ValueArray{convert(cv::viz::Color::white()), convert(cv::viz::Color::black()), convert(cv::viz::Color::gray())};
    } else {
      return ValueArray{convert(cv::viz::Color::blue()), convert(cv::viz::Color::red()), convert(cv::viz::Color::black())};
    }
  }

  static constexpr Value convert(const cv::viz::Color &color) {
    Value ret;
    if constexpr(cv::DataType<T>::channels == 1) {
      ret = color[0];
    } else {
      for(int i = 0; i < cv::DataType<T>::channels; i++) {
        ret[i] = color[i];
      }
    }
    return ret;
  }
};

/*!
\brief This class extends cv::Mat_<T> for event images. For more information, please refer <a href="https://docs.opencv.org/master/d3/d63/classcv_1_1Mat.html">here</a>.

Analogously to OpenCV library, the following aliases are defined for convenience:
\code{.cpp}
using EventImage1b = EventImage_<uchar>;
using EventImage2b = EventImage_<cv::Vec2b>;
using EventImage3b = EventImage_<cv::Vec3b>;
using EventImage4b = EventImage_<cv::Vec4b>;
using EventImage1s = EventImage_<short>;
using EventImage2s = EventImage_<cv::Vec2s>;
using EventImage3s = EventImage_<cv::Vec3s>;
using EventImage4s = EventImage_<cv::Vec4s>;
using EventImage1w = EventImage_<ushort>;
using EventImage2w = EventImage_<cv::Vec2w>;
using EventImage3w = EventImage_<cv::Vec3w>;
using EventImage4w = EventImage_<cv::Vec4w>;
using EventImage1i = EventImage_<int>;
using EventImage2i = EventImage_<cv::Vec2i>;
using EventImage3i = EventImage_<cv::Vec3i>;
using EventImage4i = EventImage_<cv::Vec4i>;
using EventImage1f = EventImage_<float>;
using EventImage2f = EventImage_<cv::Vec2f>;
using EventImage3f = EventImage_<cv::Vec3f>;
using EventImage4f = EventImage_<cv::Vec4f>;
using EventImage1d = EventImage_<double>;
using EventImage2d = EventImage_<cv::Vec2d>;
using EventImage3d = EventImage_<cv::Vec3d>;
using EventImage4d = EventImage_<cv::Vec4d>;
using EventImage1 = EventImage1b;
using EventImage3 = EventImage3b;
using EventImage = EventImage1;
\endcode
*/
template <typename T>
class EventImage_ : public AbstractRepresentation_<T> {
  using AbstractRepresentation_<T>::AbstractRepresentation_;

public:
  using Value = typename ColorHelper<T>::Value;           /*!< Event image value type */
  using ValueArray = typename ColorHelper<T>::ValueArray; /*!< Array of event image value types */

  /*! \cond INTERNAL */
  ~EventImage_() override = default;
  EventImage_(const EventImage_<T> &) = default;
  EventImage_(EventImage_<T> &&) noexcept = default;
  EventImage_<T> &operator=(const EventImage_<T> &) = default;
  EventImage_<T> &operator=(EventImage_<T> &&) noexcept = default;
  /*! \endcond */

  /*!
  \brief Set colors for ON, OFF, and non-activated pixels. For more information, please refer <a href="https://docs.opencv.org/master/d4/dba/classcv_1_1viz_1_1Color.html">here</a>.
  \param positive Color for ON pixels
  \param negative Color for OFF pixels
  \param reset Color for non-activated pixels
  \note cv::viz::Color inherits from cv::Scalar_<double>. Color are defined as BGR with values between 0 and 255.
  */
  inline void setColors(const cv::viz::Color &positive, const cv::viz::Color &negative, const cv::viz::Color &reset) {
    setValues(ColorHelper<T>::convert(positive), ColorHelper<T>::convert(negative), ColorHelper<T>::convert(reset));
  }

  /*!
  \brief Set values for ON, OFF, and non-activated pixels.
  \param positive Value for ON pixels
  \param negative Value for OFF pixels
  \param reset Value for non-activated pixels
  */
  inline void setValues(const Value &positive, const Value &negative, const Value &reset) {
    imageValues_ = {positive, negative, reset};
  }

  /*!
  \brief Set values for ON, OFF, and non-activated pixels.
  \param pnr Array with values for, respectively, ON, OFF, and non-activated pixels
  */
  inline void setValues(const ValueArray &pnr) {
    imageValues_ = pnr;
  }

  /*!
  \brief Get current values for ON, OFF, and non-activated pixels.
  \return Array with values for, respectively, ON, OFF, and non-activated pixels
  */
  [[nodiscard]] inline ValueArray getValues() const {
    return imageValues_;
  }

protected:
  /*! \cond INTERNAL */
  enum value { ON,
               OFF,
               RESET };
  ValueArray imageValues_{ColorHelper<T>::initialize()};
  /*! \endcond */

private:
  void clear_() override;
  void insert_(const Event &e) override;
};
using EventImage1b = EventImage_<uchar>;     /*!< Alias for EventImage_ using uchar */
using EventImage2b = EventImage_<cv::Vec2b>; /*!< Alias for EventImage_ using cv::Vec2b */
using EventImage3b = EventImage_<cv::Vec3b>; /*!< Alias for EventImage_ using cv::Vec3b */
using EventImage4b = EventImage_<cv::Vec4b>; /*!< Alias for EventImage_ using cv::Vec4b */
using EventImage1s = EventImage_<short>;     /*!< Alias for EventImage_ using short */
using EventImage2s = EventImage_<cv::Vec2s>; /*!< Alias for EventImage_ using cv::Vec2s */
using EventImage3s = EventImage_<cv::Vec3s>; /*!< Alias for EventImage_ using cv::Vec3s */
using EventImage4s = EventImage_<cv::Vec4s>; /*!< Alias for EventImage_ using cv::Vec4s */
using EventImage1w = EventImage_<ushort>;    /*!< Alias for EventImage_ using ushort */
using EventImage2w = EventImage_<cv::Vec2w>; /*!< Alias for EventImage_ using cv::Vec2w */
using EventImage3w = EventImage_<cv::Vec3w>; /*!< Alias for EventImage_ using cv::Vec3w */
using EventImage4w = EventImage_<cv::Vec4w>; /*!< Alias for EventImage_ using cv::Vec4w */
using EventImage1i = EventImage_<int>;       /*!< Alias for EventImage_ using int */
using EventImage2i = EventImage_<cv::Vec2i>; /*!< Alias for EventImage_ using cv::Vec2i */
using EventImage3i = EventImage_<cv::Vec3i>; /*!< Alias for EventImage_ using cv::Vec3i */
using EventImage4i = EventImage_<cv::Vec4i>; /*!< Alias for EventImage_ using cv::Vec4i */
using EventImage1f = EventImage_<float>;     /*!< Alias for EventImage_ using float */
using EventImage2f = EventImage_<cv::Vec2f>; /*!< Alias for EventImage_ using cv::Vec2f */
using EventImage3f = EventImage_<cv::Vec3f>; /*!< Alias for EventImage_ using cv::Vec3f */
using EventImage4f = EventImage_<cv::Vec4f>; /*!< Alias for EventImage_ using cv::Vec4f */
using EventImage1d = EventImage_<double>;    /*!< Alias for EventImage_ using double */
using EventImage2d = EventImage_<cv::Vec2d>; /*!< Alias for EventImage_ using cv::Vec2d */
using EventImage3d = EventImage_<cv::Vec3d>; /*!< Alias for EventImage_ using cv::Vec3d */
using EventImage4d = EventImage_<cv::Vec4d>; /*!< Alias for EventImage_ using cv::Vec4d */
using EventImage1 = EventImage1b;            /*!< Alias for EventImage_ using uchar */
using EventImage3 = EventImage3b;            /*!< Alias for EventImage_ using cv::Vec3b */
using EventImage = EventImage1;              /*!< Alias for EventImage_ using uchar */

/*!
\brief This class extends ev::EventImage_<T> for event images.

Analogously to OpenCV library, the following aliases are defined for convenience:
\code{.cpp}
using TimeSurface1b = TimeSurface_<uchar>;
using TimeSurface2b = TimeSurface_<cv::Vec2b>;
using TimeSurface3b = TimeSurface_<cv::Vec3b>;
using TimeSurface4b = TimeSurface_<cv::Vec4b>;
using TimeSurface1s = TimeSurface_<short>;
using TimeSurface2s = TimeSurface_<cv::Vec2s>;
using TimeSurface3s = TimeSurface_<cv::Vec3s>;
using TimeSurface4s = TimeSurface_<cv::Vec4s>;
using TimeSurface1w = TimeSurface_<ushort>;
using TimeSurface2w = TimeSurface_<cv::Vec2w>;
using TimeSurface3w = TimeSurface_<cv::Vec3w>;
using TimeSurface4w = TimeSurface_<cv::Vec4w>;
using TimeSurface1i = TimeSurface_<int>;
using TimeSurface2i = TimeSurface_<cv::Vec2i>;
using TimeSurface3i = TimeSurface_<cv::Vec3i>;
using TimeSurface4i = TimeSurface_<cv::Vec4i>;
using TimeSurface1f = TimeSurface_<float>;
using TimeSurface2f = TimeSurface_<cv::Vec2f>;
using TimeSurface3f = TimeSurface_<cv::Vec3f>;
using TimeSurface4f = TimeSurface_<cv::Vec4f>;
using TimeSurface1d = TimeSurface_<double>;
using TimeSurface2d = TimeSurface_<cv::Vec2d>;
using TimeSurface3d = TimeSurface_<cv::Vec3d>;
using TimeSurface4d = TimeSurface_<cv::Vec4d>;
using TimeSurface1 = TimeSurface1b;
using TimeSurface3 = TimeSurface3b;
using TimeSurface = TimeSurface1;
\endcode
*/
template <typename T>
class TimeSurface_ : public EventImage_<T> {
  using EventImage_<T>::EventImage_;

public:
  EventImage1d timestamp{this->size()}; /*!< Timestamps */
  EventImage1d polarity{this->size()};  /*!< Polarities */

  /*!
  Time-surface matrices are generated from timestamp and polarity matrices.
  \brief Render Time-surface matrix.
  \return Array with values for, respectively, ON, OFF, and non-activated pixels
  */
  void render();

private:
  void clear_() override;
  void insert_(const Event &e) override;
};
using TimeSurface1b = TimeSurface_<uchar>;     /*!< Alias for TimeSurface_ using uchar */
using TimeSurface2b = TimeSurface_<cv::Vec2b>; /*!< Alias for TimeSurface_ using cv::Vec2b */
using TimeSurface3b = TimeSurface_<cv::Vec3b>; /*!< Alias for TimeSurface_ using cv::Vec3b */
using TimeSurface4b = TimeSurface_<cv::Vec4b>; /*!< Alias for TimeSurface_ using cv::Vec4b */
using TimeSurface1s = TimeSurface_<short>;     /*!< Alias for TimeSurface_ using short */
using TimeSurface2s = TimeSurface_<cv::Vec2s>; /*!< Alias for TimeSurface_ using cv::Vec2s */
using TimeSurface3s = TimeSurface_<cv::Vec3s>; /*!< Alias for TimeSurface_ using cv::Vec3s */
using TimeSurface4s = TimeSurface_<cv::Vec4s>; /*!< Alias for TimeSurface_ using cv::Vec4s */
using TimeSurface1w = TimeSurface_<ushort>;    /*!< Alias for TimeSurface_ using ushort */
using TimeSurface2w = TimeSurface_<cv::Vec2w>; /*!< Alias for TimeSurface_ using cv::Vec2w */
using TimeSurface3w = TimeSurface_<cv::Vec3w>; /*!< Alias for TimeSurface_ using cv::Vec3w */
using TimeSurface4w = TimeSurface_<cv::Vec4w>; /*!< Alias for TimeSurface_ using cv::Vec4w */
using TimeSurface1i = TimeSurface_<int>;       /*!< Alias for TimeSurface_ using int */
using TimeSurface2i = TimeSurface_<cv::Vec2i>; /*!< Alias for TimeSurface_ using cv::Vec2i */
using TimeSurface3i = TimeSurface_<cv::Vec3i>; /*!< Alias for TimeSurface_ using cv::Vec3i */
using TimeSurface4i = TimeSurface_<cv::Vec4i>; /*!< Alias for TimeSurface_ using cv::Vec4i */
using TimeSurface1f = TimeSurface_<float>;     /*!< Alias for TimeSurface_ using float */
using TimeSurface2f = TimeSurface_<cv::Vec2f>; /*!< Alias for TimeSurface_ using cv::Vec2f */
using TimeSurface3f = TimeSurface_<cv::Vec3f>; /*!< Alias for TimeSurface_ using cv::Vec3f */
using TimeSurface4f = TimeSurface_<cv::Vec4f>; /*!< Alias for TimeSurface_ using cv::Vec4f */
using TimeSurface1d = TimeSurface_<double>;    /*!< Alias for TimeSurface_ using double */
using TimeSurface2d = TimeSurface_<cv::Vec2d>; /*!< Alias for TimeSurface_ using cv::Vec2d */
using TimeSurface3d = TimeSurface_<cv::Vec3d>; /*!< Alias for TimeSurface_ using cv::Vec3d */
using TimeSurface4d = TimeSurface_<cv::Vec4d>; /*!< Alias for TimeSurface_ using cv::Vec4d */
using TimeSurface1 = TimeSurface1b;            /*!< Alias for TimeSurface_ using uchar */
using TimeSurface3 = TimeSurface3b;            /*!< Alias for TimeSurface_ using cv::Vec3b */
using TimeSurface = TimeSurface1;              /*!< Alias for TimeSurface_ using uchar */

// TODO: Add voxel grids
// TODO: Add point sets
// TODO: Add reconstructed images

} // namespace ev

/*! \cond INTERNAL */
#include "openev/abstract-representation.tpp"
#include "openev/event-image.tpp"
#include "openev/time-surface.tpp"
/*! \endcond */

#endif // OPENEV_REPRESENTATION_HPP
