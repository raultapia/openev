/*!
\file representation.hpp
\brief Event representations.
\author Raul Tapia
*/
#ifndef OPENEV_REPRESENTATION_HPP
#define OPENEV_REPRESENTATION_HPP

#include "openev/containers.hpp"
#include "openev/types.hpp"
#include <opencv2/viz.hpp>
#include <opencv2/viz/types.hpp>
#include <utility>

namespace ev {

enum RepresentationOptions : uint8_t {
  NONE = 0b00000000,
  IGNORE_POLARITY = 0b00000001,
  ONLY_IF_POSITIVE = 0b00000010,
  ONLY_IF_NEGATIVE = 0b00000100
};
constexpr bool REPRESENTATION_OPTION_CHECK(const uint8_t a, const ev::RepresentationOptions b) {
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
        return TypeArray{convert(cv::viz::Color::white()), convert(cv::viz::Color::black()), convert(cv::viz::Color::gray())};
      } else {
        return TypeArray{convert(cv::viz::Color::blue()), convert(cv::viz::Color::red()), convert(cv::viz::Color::black())};
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
  \return True if all the events have been inserted
  */
  bool insert(Queue &queue);

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
      ON = value;
    } else {
      OFF = value;
    }
  }

  /*!
  \brief Set value for non-activated pixels (background).
  \param value Value for non-activated pixels
  */
  inline void setValue(const Type &value) {
    RESET = value;
  }

  /*!
  \brief Set values for ON, OFF, and non-activated pixels.
  \param positive Value for ON pixels
  \param negative Value for OFF pixels
  \param reset Value for non-activated pixels
  */
  inline void setValues(const Type &positive, const Type &negative, const Type &reset) {
    ON = positive;
    OFF = negative;
    RESET = reset;
  }

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

  /*!
  \brief Get current values for ON and OFF pixels.
  \param polarity Positive (ON) or negative (OFF)
  \return Values for ON/OFF pixels
  */
  [[nodiscard]] inline Type getValue(const bool polarity) const {
    return polarity ? ON : OFF;
  }

  /*!
  \brief Get current values for non-activated pixels (background).
  \return Values for non-activated pixels
  */
  [[nodiscard]] inline Type getValue() const {
    return RESET;
  }

protected:
  /*! \cond INTERNAL */
  enum : uint8_t { MIN,
                   MAX };

  Type ON = TypeHelper<T>::initialize()[0];
  Type OFF = TypeHelper<T>::initialize()[1];
  Type RESET = TypeHelper<T>::initialize()[2];

  double timeOffset_{0};
  std::array<double, 2> tLimits_{DBL_MAX, DBL_MIN};
  std::size_t count_{0};

  virtual void clear_() = 0;
  virtual bool insert_(const Event &e) = 0;
  /*! \endcond */
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
template <typename T, const RepresentationOptions Options = RepresentationOptions::NONE>
class EventImage_ : public cv::Mat_<T>, public AbstractRepresentation_<T, Options> {
public:
  template <typename... Args>
  explicit EventImage_(Args &&...args) : cv::Mat_<T>(std::forward<Args>(args)...) {
    AbstractRepresentation_<T, Options>::clear();
  }

private:
  void clear_() override;
  bool insert_(const Event &e) override;
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
\brief This class extends ev::EventImage_<T> for event 2D histograms.

Analogously to OpenCV library, the following aliases are defined for convenience:
\code{.cpp}
using EventHistogram1b = EventHistogram_<uchar>;
using EventHistogram2b = EventHistogram_<cv::Vec2b>;
using EventHistogram3b = EventHistogram_<cv::Vec3b>;
using EventHistogram4b = EventHistogram_<cv::Vec4b>;
using EventHistogram1s = EventHistogram_<short>;
using EventHistogram2s = EventHistogram_<cv::Vec2s>;
using EventHistogram3s = EventHistogram_<cv::Vec3s>;
using EventHistogram4s = EventHistogram_<cv::Vec4s>;
using EventHistogram1w = EventHistogram_<ushort>;
using EventHistogram2w = EventHistogram_<cv::Vec2w>;
using EventHistogram3w = EventHistogram_<cv::Vec3w>;
using EventHistogram4w = EventHistogram_<cv::Vec4w>;
using EventHistogram1i = EventHistogram_<int>;
using EventHistogram2i = EventHistogram_<cv::Vec2i>;
using EventHistogram3i = EventHistogram_<cv::Vec3i>;
using EventHistogram4i = EventHistogram_<cv::Vec4i>;
using EventHistogram1f = EventHistogram_<float>;
using EventHistogram2f = EventHistogram_<cv::Vec2f>;
using EventHistogram3f = EventHistogram_<cv::Vec3f>;
using EventHistogram4f = EventHistogram_<cv::Vec4f>;
using EventHistogram1d = EventHistogram_<double>;
using EventHistogram2d = EventHistogram_<cv::Vec2d>;
using EventHistogram3d = EventHistogram_<cv::Vec3d>;
using EventHistogram4d = EventHistogram_<cv::Vec4d>;
using EventHistogram1 = EventHistogram1b;
using EventHistogram3 = EventHistogram3b;
using EventHistogram = EventHistogram1;
\endcode
*/
template <typename T, const RepresentationOptions Options = RepresentationOptions::NONE>
class EventHistogram_ : public EventImage_<T, Options> {
public:
  template <typename... Args>
  explicit EventHistogram_(Args &&...args) : EventImage_<T, Options>(std::forward<Args>(args)...) {
    EventImage_<T, Options>::clear();
  }

  std::array<cv::Mat_<int>, 2> counter{
      cv::Mat_<int>(EventImage_<T, Options>::size()),
      cv::Mat_<int>(EventImage_<T, Options>::size())}; /*!< Event counter */

  /*!
  Event histogram matrix is generated from counter matrix.
  \brief Render event histogram matrix.
  */
  void render();

private:
  void clear_() override;
  bool insert_(const Event &e) override;
};
using EventHistogram1b = EventHistogram_<uchar>;     /*!< Alias for EventHistogram_ using uchar */
using EventHistogram2b = EventHistogram_<cv::Vec2b>; /*!< Alias for EventHistogram_ using cv::Vec2b */
using EventHistogram3b = EventHistogram_<cv::Vec3b>; /*!< Alias for EventHistogram_ using cv::Vec3b */
using EventHistogram4b = EventHistogram_<cv::Vec4b>; /*!< Alias for EventHistogram_ using cv::Vec4b */
using EventHistogram1s = EventHistogram_<short>;     /*!< Alias for EventHistogram_ using short */
using EventHistogram2s = EventHistogram_<cv::Vec2s>; /*!< Alias for EventHistogram_ using cv::Vec2s */
using EventHistogram3s = EventHistogram_<cv::Vec3s>; /*!< Alias for EventHistogram_ using cv::Vec3s */
using EventHistogram4s = EventHistogram_<cv::Vec4s>; /*!< Alias for EventHistogram_ using cv::Vec4s */
using EventHistogram1w = EventHistogram_<ushort>;    /*!< Alias for EventHistogram_ using ushort */
using EventHistogram2w = EventHistogram_<cv::Vec2w>; /*!< Alias for EventHistogram_ using cv::Vec2w */
using EventHistogram3w = EventHistogram_<cv::Vec3w>; /*!< Alias for EventHistogram_ using cv::Vec3w */
using EventHistogram4w = EventHistogram_<cv::Vec4w>; /*!< Alias for EventHistogram_ using cv::Vec4w */
using EventHistogram1i = EventHistogram_<int>;       /*!< Alias for EventHistogram_ using int */
using EventHistogram2i = EventHistogram_<cv::Vec2i>; /*!< Alias for EventHistogram_ using cv::Vec2i */
using EventHistogram3i = EventHistogram_<cv::Vec3i>; /*!< Alias for EventHistogram_ using cv::Vec3i */
using EventHistogram4i = EventHistogram_<cv::Vec4i>; /*!< Alias for EventHistogram_ using cv::Vec4i */
using EventHistogram1f = EventHistogram_<float>;     /*!< Alias for EventHistogram_ using float */
using EventHistogram2f = EventHistogram_<cv::Vec2f>; /*!< Alias for EventHistogram_ using cv::Vec2f */
using EventHistogram3f = EventHistogram_<cv::Vec3f>; /*!< Alias for EventHistogram_ using cv::Vec3f */
using EventHistogram4f = EventHistogram_<cv::Vec4f>; /*!< Alias for EventHistogram_ using cv::Vec4f */
using EventHistogram1d = EventHistogram_<double>;    /*!< Alias for EventHistogram_ using double */
using EventHistogram2d = EventHistogram_<cv::Vec2d>; /*!< Alias for EventHistogram_ using cv::Vec2d */
using EventHistogram3d = EventHistogram_<cv::Vec3d>; /*!< Alias for EventHistogram_ using cv::Vec3d */
using EventHistogram4d = EventHistogram_<cv::Vec4d>; /*!< Alias for EventHistogram_ using cv::Vec4d */
using EventHistogram1 = EventHistogram1b;            /*!< Alias for EventHistogram_ using uchar */
using EventHistogram3 = EventHistogram3b;            /*!< Alias for EventHistogram_ using cv::Vec3b */
using EventHistogram = EventHistogram1;              /*!< Alias for EventHistogram_ using uchar */

/*!
\brief This class extends ev::EventImage_<T> for time surfaces.

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
enum class Kernel { NONE,
                    LINEAR,
                    EXPONENTIAL };
template <typename T, const RepresentationOptions Options = RepresentationOptions::NONE>
class TimeSurface_ : public EventImage_<T, Options> {
public:
  template <typename... Args>
  explicit TimeSurface_(Args &&...args) : EventImage_<T, Options>(std::forward<Args>(args)...) {
    EventImage_<T, Options>::clear();
  }

  cv::Mat_<double> time{this->size()};   /*!< Time matrix */
  cv::Mat_<bool> polarity{this->size()}; /*!< Polarity matrix */

  /*!
  Timesurface matrix is generated from timestamp and polarity matrices.
  \brief Render timesurface matrix.
  \param kernel Kernel type
  \param tau Time constant
  \see TimeSurface_::Kernel
  */
  void render(const Kernel kernel = Kernel::NONE, const double tau = 0);

private:
  void clear_() override;
  bool insert_(const Event &e) override;
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

/*!
\brief This class is used to represent events as point clouds.

Analogously to OpenCV library, the following aliases are defined for convenience:
\code{.cpp}
using PointCloud1b = PointCloud_<uchar>;
using PointCloud3b = PointCloud_<cv::Vec3b>;
using PointCloud1s = PointCloud_<short>;
using PointCloud3s = PointCloud_<cv::Vec3s>;
using PointCloud1w = PointCloud_<ushort>;
using PointCloud3w = PointCloud_<cv::Vec3w>;
using PointCloud1i = PointCloud_<int>;
using PointCloud3i = PointCloud_<cv::Vec3i>;
using PointCloud1f = PointCloud_<float>;
using PointCloud3f = PointCloud_<cv::Vec3f>;
using PointCloud1d = PointCloud_<double>;
using PointCloud3d = PointCloud_<cv::Vec3d>;
using PointCloud1 = PointCloud1b;
using PointCloud3 = PointCloud3b;
using PointCloud = PointCloud1;
\endcode
*/
template <typename T, const RepresentationOptions Options = RepresentationOptions::NONE>
class PointCloud_ : public AbstractRepresentation_<T, Options> {
public:
  /*!
  Default contructor.
  */
  PointCloud_() : frame_{0, 0}, scaleFactor_{1.0} {};

  /*!
  Contructor using frame size.
  \note If no size selected, frame bounds are not checked when inserting
  \param height Height of the frame
  \param width Width of the frame
  \param scale_factor Scale factor for time axis
  */
  PointCloud_(const int height, const int width, const double scale_factor) : frame_{width, height}, scaleFactor_{scale_factor} {};

  /*!
  Contructor using frame size.
  \note If no size selected, frame bounds are not checked when inserting
  \param height Height of the frame
  \param width Width of the frame
  */
  PointCloud_(const int height, const int width) : frame_{width, height}, scaleFactor_{1.0} {};

  /*!
  Contructor using frame size.
  \note If no size selected, frame bounds are not checked when inserting
  \param size Frame size
  \param scale_factor Scale factor for time axis
  */
  explicit PointCloud_(const Size size, const double scale_factor) : frame_{size}, scaleFactor_{scale_factor} {};

  /*!
  Contructor using frame size.
  \note If no size selected, frame bounds are not checked when inserting
  \param size Frame size
  */
  explicit PointCloud_(const Size size) : frame_{size}, scaleFactor_{1.0} {};

  /*!
  \brief Set frame size.
  \note If no size selected, frame bounds are not checked when inserting
  \param height Height of the frame
  \param width Width of the frame
  */
  inline void setFrameSize(const int height, const int width) {
    frame_ = {width, height};
  }

  /*!
  \brief Set frame size.
  \note If no size selected, frame bounds are not checked when inserting
  \param size Frame size
  */
  inline void setFrameSize(const Size size) {
    frame_ = size;
  }

  /*!
  \brief Get frame size.
  \return Frame size
  */
  [[nodiscard]] inline Size getFrameSize() const {
    return frame_;
  }

  /*!
  \brief Check if an event is included in the point cloud.
  \param e Event to check
  */
  [[nodiscard]] inline bool contains(const Event &e) const {
    return std::find(points_[e.p].begin(), points_[e.p].end(), cv::Point3_<BasicDataType<T>>(e.x, e.y, e.t)) != points_[e.p].end();
  }

  /*!
  \brief Visualize point cloud
  */
  void visualizeOnce();

  /*!
  \brief Visualize point cloud
  */
  void visualize();

private:
  std::array<std::vector<cv::Point3_<BasicDataType<T>>>, 2> points_;
  cv::viz::Viz3d window_{"OpenEV"};
  ev::Size frame_;
  const double scaleFactor_;

  void clear_() override;
  bool insert_(const Event &e) override;
};
using PointCloud1b = PointCloud_<uchar>;     /*!< Alias for PointCloud_ using uchar */
using PointCloud3b = PointCloud_<cv::Vec3b>; /*!< Alias for PointCloud_ using cv::Vec3b */
using PointCloud1s = PointCloud_<short>;     /*!< Alias for PointCloud_ using short */
using PointCloud3s = PointCloud_<cv::Vec3s>; /*!< Alias for PointCloud_ using cv::Vec3s */
using PointCloud1w = PointCloud_<ushort>;    /*!< Alias for PointCloud_ using ushort */
using PointCloud3w = PointCloud_<cv::Vec3w>; /*!< Alias for PointCloud_ using cv::Vec3w */
using PointCloud1i = PointCloud_<int>;       /*!< Alias for PointCloud_ using int */
using PointCloud3i = PointCloud_<cv::Vec3i>; /*!< Alias for PointCloud_ using cv::Vec3i */
using PointCloud1f = PointCloud_<float>;     /*!< Alias for PointCloud_ using float */
using PointCloud3f = PointCloud_<cv::Vec3f>; /*!< Alias for PointCloud_ using cv::Vec3f */
using PointCloud1d = PointCloud_<double>;    /*!< Alias for PointCloud_ using double */
using PointCloud3d = PointCloud_<cv::Vec3d>; /*!< Alias for PointCloud_ using cv::Vec3d */
using PointCloud1 = PointCloud1b;            /*!< Alias for PointCloud_ using uchar */
using PointCloud3 = PointCloud3b;            /*!< Alias for PointCloud_ using cv::Vec3b */
using PointCloud = PointCloud1;              /*!< Alias for PointCloud_ using uchar */

// TODO: Add voxel grids
// TODO: Add reconstructed images

} // namespace ev

/*! \cond INTERNAL */
#include "openev/abstract-representation.tpp"
#include "openev/event-histogram.tpp"
#include "openev/event-image.tpp"
#include "openev/point-cloud.tpp"
#include "openev/time-surface.tpp"
/*! \endcond */

#endif // OPENEV_REPRESENTATION_HPP
