/*!
\file types.hpp
\brief Basic event-based vision structures based on OpenCV components.
\author Raul Tapia
*/
#ifndef OPENEV_CORE_TYPES_HPP
#define OPENEV_CORE_TYPES_HPP

#include "openev/core/undistortion.hpp"
#include "openev/utils/logger.hpp"
#include <cmath>
#include <cstdint>
#include <iosfwd>
#include <opencv2/core/base.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/types.hpp>
#include <string>
#include <vector>

namespace ev {
constexpr bool POSITIVE = true;  /*!< Positive polarity */
constexpr bool NEGATIVE = false; /*!< Negative polarity */

enum class Stereo : char {
  LEFT = 'L',
  RIGHT = 'R'
};

enum DistanceTypes : uint8_t {
  DISTANCE_NORM_INF = cv::NORM_INF,
  DISTANCE_NORM_L1 = cv::NORM_L1,
  DISTANCE_NORM_L2 = cv::NORM_L2,
  DISTANCE_NORM_L2SQR = cv::NORM_L2SQR,
  DISTANCE_NORM_MANHATTAN = DISTANCE_NORM_L1,
  DISTANCE_NORM_EUCLIDEAN = DISTANCE_NORM_L2,
  DISTANCE_FLAG_SPATIAL = 0b00010000,
  DISTANCE_FLAG_TEMPORAL = 0b00100000,
  DISTANCE_FLAG_SPATIOTEMPORAL = 0b01000000,
  DISTANCE_FLAG_3D = DISTANCE_FLAG_SPATIOTEMPORAL,
  DISTANCE_FLAG_2D = DISTANCE_FLAG_SPATIAL,
};

/*!
\brief This class extends cv::Point_<T> for event data. For more information, please refer <a href="https://docs.opencv.org/master/db/d4e/classcv_1_1Point__.html">here</a>.

Analogously to OpenCV library, the following aliases are defined for convenience:
\code{.cpp}
using Eventi = Event_<int>;
using Eventl = Event_<long>;
using Eventf = Event_<float>;
using Eventd = Event_<double>;
using Event = Eventi;
\endcode
*/
template <typename T>
class Event_ : public cv::Point_<T> {
public:
  double t; /*!< Event timestamp */
  bool p;   /*!< Event polarity */

  /*!
  Default constructor.
  */
  Event_() : cv::Point_<T>(), t{0}, p{POSITIVE} {};

  /*! \cond INTERNAL */
  ~Event_() = default;
  /*! \endcond */

  /*!
  Copy constructor.
  */
  Event_(const Event_<T> &) = default;

  /*!
  Move constructor.
  */
  Event_(Event_<T> &&) noexcept = default;

  /*!
  Contructor using event coordinates.
  \param x Spatial coordinate x
  \param y Spatial coordinate y
  */
  Event_(const T x, const T y) : cv::Point_<T>(x, y), t{0}, p{POSITIVE} {};

  /*!
  Contructor using event coordinates as cv::Point.
  \param pt Spatial coordinate (x,y) as point
  */
  explicit Event_(const cv::Point_<T> &pt) : cv::Point_<T>(pt), t{0}, p{POSITIVE} {};

  /*!
  Contructor using timestamp, event coordinates, and polarity.
  \param x Spatial coordinate x
  \param y Spatial coordinate y
  \param t Timestamp
  \param p Polarity
  */
  Event_(const T x, const T y, const double t, const bool p) : cv::Point_<T>(x, y), t{t}, p{p} {};

  /*!
  Contructor using timestamp, event coordinates as cv::Point, and polarity.
  \param pt Spatial coordinate (x,y) as point
  \param t Timestamp
  \param p Polarity
  */
  Event_(const cv::Point_<T> &pt, const double t, const bool p) : cv::Point_<T>(pt), t{t}, p{p} {};

  /*!
  Copy assignment operator
  */
  Event_<T> &operator=(const Event_<T> &) = default;

  /*!
  Copy assignment operator
  */
  Event_<T> &operator=(const cv::Point_<T> &p) {
    Event_<T>::x = p.x;
    Event_<T>::y = p.y;
    return *this;
  }

  /*!
  Move assignment operator
  */
  Event_<T> &operator=(Event_<T> &&) noexcept = default;

  /*!
  Equality operator
  */
  [[nodiscard]] inline bool operator==(const Event_<T> &e) const {
    return (Event_<T>::x == e.x) && (Event_<T>::y == e.y) && (t == e.t) && (p == e.p);
  }

  /*!
  cv::Point equality operator
  */
  [[nodiscard]] inline bool operator==(const cv::Point_<T> &pt) const {
    return (Event_<T>::x == pt.x) && (Event_<T>::y == pt.y);
  }

  /*!
  cv::Point3 equality operator
  */
  [[nodiscard]] inline bool operator==(const cv::Point3_<T> &pt) const {
    return (Event_<T>::x == pt.x) && (Event_<T>::y == pt.y) && (Event_<T>::t == pt.z);
  }

  /*!
  Comparison operator
  */
  [[nodiscard]] inline bool operator<(const Event_<T> &e) const {
    return Event_<T>::t < e.t;
  }

  /*!
  cv::Point cast operator
  */
  template <typename U>
  [[nodiscard]] inline operator cv::Point_<U>() const {
    return {static_cast<U>(Event_<T>::x), static_cast<U>(Event_<T>::y)};
  }

  /*!
  cv::Point3 cast operator
  */
  template <typename U>
  [[nodiscard]] inline operator cv::Point3_<U>() const {
    return {static_cast<U>(Event_<T>::x), static_cast<U>(Event_<T>::y), static_cast<U>(Event_<T>::t)};
  }

  /*!
  Compute distance between events.
  \param e Other event
  \param type Distance type
  \return Distance
  \see Distance
  */
  [[nodiscard]] inline double distance(const Event_<T> &e, const uint8_t type = DISTANCE_NORM_L2 | DISTANCE_FLAG_SPATIAL) const {
    if(static_cast<bool>(type & DISTANCE_FLAG_SPATIOTEMPORAL)) {
      return cv::norm(cv::Matx<T, 3, 1>(Event_<T>::x - e.x, Event_<T>::y - e.y, Event_<T>::t - e.t), type & 0x0F);
    }
    if(static_cast<bool>(type & DISTANCE_FLAG_SPATIAL) || !static_cast<bool>(type & 0xF0)) {
      return cv::norm(cv::Matx<T, 2, 1>(Event_<T>::x - e.x, Event_<T>::y - e.y), type & 0x0F);
    }
    if(static_cast<bool>(type & DISTANCE_FLAG_TEMPORAL)) {
      return Event_<T>::t - e.t;
    }
    logger::error("Bad distance option");
    return 0.0;
  }

  /*!
  Undistort event using undistortion map
  \param map Undistortion map
  \return True if the new coordinates lie on the frame
  \see UndistortMap
  */
  inline bool undistort(const UndistortMap &map) {
    return map(*this);
  }

  /*!
  \brief Bilinear voting.
  \return Vector of weights
  */
  std::array<double, 4> bilinearVoting() {
    const double dx = this->x - static_cast<int>(this->x);
    const double dy = this->y - static_cast<int>(this->y);
    if(dx == 0 && dy == 0) {
      return {1, 0, 0, 0};
    }
    if(dx == 0) {
      return {(1 - dy), 0, dy, 0};
    }
    if(dy == 0) {
      return {(1 - dx), dx, 0, 0};
    }
    return {(1 - dx) * (1 - dy), dx * (1 - dy), (1 - dx) * dy, dx * dy};
  }

  /*!
  \brief Overload of << operator.
  \param os Output stream
  \param e Event to print
  \return Output stream
  */
  [[nodiscard]] friend std::ostream &operator<<(std::ostream &os, const Event_<T> &e) {
    os << std::string("(" + std::to_string(e.x) + "," + std::to_string(e.y) + ") " + std::to_string(e.t) + (e.p ? " [+]" : " [-]"));
    return os;
  }
};
using Eventi = Event_<int>;    /*!< Alias for Event_ using int */
using Eventl = Event_<long>;   /*!< Alias for Event_ using long */
using Eventf = Event_<float>;  /*!< Alias for Event_ using float */
using Eventd = Event_<double>; /*!< Alias for Event_ using double */
using Event = Eventi;          /*!< Alias for Event_ using int */

/*!
\brief This class extends Event to include: weigth, depth, and stereo.

Analogously to OpenCV library, the following aliases are defined for convenience:
\code{.cpp}
using AugmentedEventi = AugmentedEvent_<int>;
using AugmentedEventl = AugmentedEvent_<long>;
using AugmentedEventf = AugmentedEvent_<float>;
using AugmentedEventd = AugmentedEvent_<double>;
using AugmentedEvent = AugmentedEventi;
\endcode
*/
template <typename T>
class AugmentedEvent_ : public Event_<T> {
  using Event_<T>::Event_;

public:
  double weight{1};            /*!< Event weight */
  double depth{0};             /*!< Event depth */
  Stereo stereo{Stereo::LEFT}; /*!< Left/right */

  /*!
  \brief Bilinear voting.
  \return Vector of events after integer casting
  */
  std::vector<AugmentedEvent_<int>> bilinearVoting() {
    const double dx = this->x - static_cast<int>(this->x);
    const double dy = this->y - static_cast<int>(this->y);
    if(dx == 0 && dy == 0) {
      return {static_cast<AugmentedEvent_<int>>(*this)};
    }
    if(dx == 0) {
      AugmentedEvent_<int> e1(static_cast<int>(this->x), static_cast<int>(this->y), this->t, this->p);
      AugmentedEvent_<int> e3(static_cast<int>(this->x), 1 + static_cast<int>(this->y), this->t, this->p);
      e1.weight = (1 - dy);
      e3.weight = dy;
      return {e1, e3};
    }
    if(dy == 0) {
      AugmentedEvent_<int> e1(static_cast<int>(this->x), static_cast<int>(this->y), this->t, this->p);
      AugmentedEvent_<int> e2(1 + static_cast<int>(this->x), static_cast<int>(this->y), this->t, this->p);
      e1.weight = (1 - dx);
      e2.weight = dx;
      return {e1, e2};
    }
    AugmentedEvent_<int> e1(static_cast<int>(this->x), static_cast<int>(this->y), this->t, this->p);
    AugmentedEvent_<int> e2(1 + static_cast<int>(this->x), static_cast<int>(this->y), this->t, this->p);
    AugmentedEvent_<int> e3(static_cast<int>(this->x), 1 + static_cast<int>(this->y), this->t, this->p);
    AugmentedEvent_<int> e4(1 + static_cast<int>(this->x), 1 + static_cast<int>(this->y), this->t, this->p);
    e1.weight = (1 - dx) * (1 - dy);
    e2.weight = dx * (1 - dy);
    e3.weight = (1 - dx) * dy;
    e4.weight = dx * dy;
    return {e1, e2, e3, e4};
  }

  /*!
  \brief Overload of << operator.
  \param os Output stream
  \param e Event to print
  \return Output stream
  */
  [[nodiscard]] friend std::ostream &operator<<(std::ostream &os, const AugmentedEvent_<T> &e) {
    os << std::string("(" + std::to_string(e.x) + "," + std::to_string(e.y) + ") " + std::to_string(e.t) + (e.p ? " [+]" : " [-]"));
    os << " w=" + std::to_string(e.weight);
    os << " d=" + std::to_string(e.depth);
    os << " s=" + (e.stereo == Stereo::LEFT ? std::string("LEFT") : std::string("RIGHT"));
    return os;
  }
};
using AugmentedEventi = AugmentedEvent_<int>;    /*!< Alias for AugmentedEvent_ using int */
using AugmentedEventl = AugmentedEvent_<long>;   /*!< Alias for AugmentedEvent_ using long */
using AugmentedEventf = AugmentedEvent_<float>;  /*!< Alias for AugmentedEvent_ using float */
using AugmentedEventd = AugmentedEvent_<double>; /*!< Alias for AugmentedEvent_ using double */
using AugmentedEvent = AugmentedEventi;          /*!< Alias for AugmentedEvent_ using int */

/*!
\brief This class extends cv::Size_<T> for event data. For more information, please refer <a href="https://docs.opencv.org/master/d6/d50/classcv_1_1Size__.html">here</a>.

Analogously to OpenCV library, the following aliases are defined for convenience:
\code{.cpp}
using Size2i = Size2_<int>;
using Size2l = Size2_<long>;
using Size2f = Size2_<float>;
using Size2d = Size2_<double>;
using Size2 = Size2i;
using Size = Size2;
\endcode
*/
template <typename T>
using Size2_ = cv::Size_<T>;
using Size2i = Size2_<int>;    /*!< Alias for Size2_ using int */
using Size2l = Size2_<long>;   /*!< Alias for Size2_ using long */
using Size2f = Size2_<float>;  /*!< Alias for Size2_ using float */
using Size2d = Size2_<double>; /*!< Alias for Size2_ using double */
using Size2 = Size2i;          /*!< Alias for Size2_ using int */
using Size = Size2;            /*!< Alias for Size2_ using int */

/*!
\brief This class extends cv::Size_<T> for event data. For more information, please refer <a href="https://docs.opencv.org/master/d6/d50/classcv_1_1Size__.html">here</a>.

Analogously to OpenCV library, the following aliases are defined for convenience:
\code{.cpp}
using Size3i = Size3_<int>;
using Size3l = Size3_<long>;
using Size3f = Size3_<float>;
using Size3d = Size3_<double>;
using Size3 = Size3i;
\endcode
*/
template <typename T>
class Size3_ : public cv::Size_<T> {
public:
  T length; /*!< Temporal dimension */

  /*!
  Default constructor.
  */
  Size3_() : cv::Size_<T>(), length{0} {};

  /*!
  Contructor using width, height, and length.
  \param w Width
  \param h Height
  \param l length (time)
  */
  Size3_(T w, T h, T l) : cv::Size_<T>(w, h), length{l} {};

  /*!
  \brief Check if empty.
  \return True if empty
  */
  [[nodiscard]] inline bool empty() const {
    return !(cv::Size_<T>::width || cv::Size_<T>::height || length);
  }

  /*!
  Volume is computed as \f$ w \cdot h \cdot l \f$.
  \brief Compute the volume.
  \return Volume
  */
  [[nodiscard]] inline T volume() const {
    return cv::Size_<T>::width * cv::Size_<T>::height * length;
  }
};
using Size3i = Size3_<int>;    /*!< Alias for Size3_ using int */
using Size3l = Size3_<long>;   /*!< Alias for Size3_ using long */
using Size3f = Size3_<float>;  /*!< Alias for Size3_ using float */
using Size3d = Size3_<double>; /*!< Alias for Size3_ using double */
using Size3 = Size3i;          /*!< Alias for Size3_ using int */

/*!
\brief This class extends cv::Rect_<T> for event data. For more information, please refer <a href="https://docs.opencv.org/master/d2/d44/classcv_1_1Rect__.html">here</a>.

Analogously to OpenCV library, the following aliases are defined for convenience:
\code{.cpp}
using Rect2i = Rect2_<int>;
using Rect2l = Rect2_<long>;
using Rect2f = Rect2_<float>;
using Rect2d = Rect2_<double>;
using Rect2 = Rect2i;
using Rect = Rect2;
\endcode
*/
template <typename T>
using Rect2_ = cv::Rect_<T>;
using Rect2i = Rect2_<int>;    /*!< Alias for Rect2_ using int */
using Rect2l = Rect2_<long>;   /*!< Alias for Rect2_ using long */
using Rect2f = Rect2_<float>;  /*!< Alias for Rect2_ using float */
using Rect2d = Rect2_<double>; /*!< Alias for Rect2_ using double */
using Rect2 = Rect2i;          /*!< Alias for Rect2_ using int */
using Rect = Rect2;            /*!< Alias for Rect2_ using int */

/*!
\brief This class extends cv::Rect_<T> for event data. For more information, please refer <a href="https://docs.opencv.org/master/d2/d44/classcv_1_1Rect__.html">here</a>.

Analogously to OpenCV library, the following aliases are defined for convenience:
\code{.cpp}
using Rect3i = Rect3_<int>;
using Rect3l = Rect3_<long>;
using Rect3f = Rect3_<float>;
using Rect3d = Rect3_<double>;
using Rect3 = Rect3i;
\endcode
*/
template <typename T>
class Rect3_ : public cv::Rect_<T> {
public:
  T t;      /*!< Temporal origin */
  T length; /*!< Temporal length */

  /*!
  Default constructor.
  */
  Rect3_() : cv::Rect_<T>(), t{0}, length{0} {};

  /*!
  Contructor using x, y, t, width, height, and length.
  \param x Origin spatial coordinate x
  \param y Origin spatial coordinate y
  \param t Origin temporal coordinate t
  \param w Width
  \param h Height
  \param l Length (time)
  */
  Rect3_(const T x, const T y, const T t, const T w, const T h, const T l) : cv::Rect_<T>(x, y, w, h), t{t}, length{l} {};

  /*!
  Contructor using rectangle, t, and length.
  \param rect Spatial position as a rectangle
  \param t Origin temporal coordinate t
  \param l Length (time)
  */
  Rect3_(const Rect2_<T> &rect, const T t, const T l) : cv::Rect_<T>(rect), t{t}, length{l} {};

  /*!
  Contructor using 3D point and 3D size.
  \param pt Origin point
  \param sz Size
  */
  Rect3_(const cv::Point3_<T> &pt, const Size3_<T> sz) : cv::Rect_<T>(cv::Point_<T>(pt.x, pt.y), cv::Size_<T>(sz.width, sz.height)), t{pt.z}, length{sz.length} {};

  /*!
  Contructor using two 3D points.
  \param pt1 Origin point
  \param pt2 Farthest point
  */
  Rect3_(const cv::Point3_<T> &pt1, const cv::Point3_<T> &pt2) : cv::Rect_<T>(cv::Point_<T>(pt1.x, pt1.y), cv::Size_<T>(pt2.x - pt1.x, pt2.y - pt1.y)), t{pt1.z}, length{pt2.z - pt1.z} {};

  /*!
  \brief Check if empty.
  \return True if empty
  */
  [[nodiscard]] inline bool empty() const {
    return !(Rect3_<T>::width && Rect3_<T>::height && length);
  }

  /*!
  \brief Check if the rectangular cuboid contains an event.
  \param e Event to check
  \return True if the event is inside
  \warning OpenCV typically assumes that the top and left boundary of the rectangle are inclusive, while the right and bottom are not.
  */
  template <typename Te>
  [[nodiscard]] inline bool contains(const Event_<Te> &e) const {
    return cv::Rect_<T>::contains(e) && e.t >= t && e.t < t + length;
  }

  /*!
  \brief Size of the rectangular cuboid.
  \return Size
  */
  [[nodiscard]] inline Size3_<T> size() const {
    return Size3_<T>(Rect3_<T>::width, Rect3_<T>::height, length);
  }

  /*!
  Volume is computed as \f$ w \cdot h \cdot l \f$.
  \brief Compute the volume.
  \return Volume
  */
  [[nodiscard]] inline T volume() const {
    return Rect3_<T>::width * Rect3_<T>::height * length;
  }
};
using Rect3i = Rect3_<int>;    /*!< Alias for Rect3_ using int */
using Rect3l = Rect3_<long>;   /*!< Alias for Rect3_ using long */
using Rect3f = Rect3_<float>;  /*!< Alias for Rect3_ using float */
using Rect3d = Rect3_<double>; /*!< Alias for Rect3_ using double */
using Rect3 = Rect3i;          /*!< Alias for Rect3_ using int */

/*!
\brief This class defines a circle given its center and radius.

Analogously to OpenCV library, the following aliases are defined for convenience:
\code{.cpp}
using Circi = Circ_<int>;
using Circl = Circ_<long>;
using Circf = Circ_<float>;
using Circd = Circ_<double>;
using Circ = Circi;
\endcode
*/
template <typename T>
struct Circ_ {
  cv::Point_<T> center;
  T radius;

  /*!
  Default constructor.
  */
  Circ_() : center{0, 0}, radius{0} {};

  /*!
  Constructor using center and radius.
  \param center Center
  \param radius Radius
  */
  Circ_(const cv::Point_<T> center, const T radius) : center{center}, radius{radius} {}

  /*!
  \brief Check if empty.
  \return True if empty
  */
  [[nodiscard]] inline bool empty() const {
    return !radius;
  }

  /*!
  \brief Check if the circle contains an event.
  \param e Event to check
  \return True if the event is inside
  \warning This function assumes that the boundary of the circle is inclusive.
  */
  template <typename Te>
  [[nodiscard]] inline bool contains(const Event_<Te> &e) const {
    return pow(center.x - e.x, 2) + pow(center.y - e.y, 2) <= radius * radius;
  }

  [[nodiscard]] inline cv::Size size() const {
    return {radius, radius};
  }

  [[nodiscard]] inline double area() const {
    return M_PI * radius * radius;
  }
};
using Circi = Circ_<int>;    /*!< Alias for Circ_ using int */
using Circl = Circ_<long>;   /*!< Alias for Circ_ using long */
using Circf = Circ_<float>;  /*!< Alias for Circ_ using float */
using Circd = Circ_<double>; /*!< Alias for Circ_ using double */
using Circ = Circi;          /*!< Alias for Circ_ using int */

} // namespace ev

#endif // OPENEV_CORE_TYPES_HPP
