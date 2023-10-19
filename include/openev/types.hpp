/*!
\file types.hpp
\brief Basic event-based vision structures based on OpenCV components.
\author Raul Tapia
*/
#ifndef OPENEV_TYPES_HPP
#define OPENEV_TYPES_HPP

#include <opencv2/core/core.hpp>
#include <queue>

namespace ev {
constexpr bool POSITIVE = true;  /*!< Positive polarity */
constexpr bool NEGATIVE = false; /*!< Negative polarity */

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
  Move assignment operator
  */
  Event_<T> &operator=(Event_<T> &&) noexcept = default;

  /*!
  Equality operator
  */
  [[nodiscard]] inline bool operator==(const Event_<T> &e) {
    return (Event_<T>::x == e.x) && (Event_<T>::y == e.y) && (t == e.t) && (p == e.p);
  }

  /*!
  cv::Point cast operator
  */
  [[nodiscard]] operator cv::Point_<T>() const {
    return {Event_<T>::x, Event_<T>::y};
  }

  /*!
  Get coordinates as cv::Point
  */
  [[nodiscard]] inline cv::Point_<T> pt() const {
    return {Event_<T>::x, Event_<T>::y};
  }

  /*!
  Given an event \f$ e_2 = (t_2, x_2, y_2, p_2) \f$, returns \f$ \sqrt{(x-x_2)^2 + (y-y_2)^2} \f$.
  \brief Euclidean distance in spatial domain w.r.t. other event.
  \param e The other event, i.e., \f$ e_2 \f$
  \return Euclidean distance
  */
  [[nodiscard]] inline double spaceDistance(const Event_<T> &e) const {
    return sqrt(pow(Event_<T>::x - e.x, 2) + pow(Event_<T>::y - e.y, 2));
  }

  /*!
  Given an event \f$ e_2 = (t_2, x_2, y_2, p_2) \f$, returns \f$ t - t_2 \f$.
  \brief Temporal difference w.r.t. other event.
  \param e The other event, i.e., \f$ e_2 \f$
  \return Temporal difference
  */
  [[nodiscard]] inline double timeDifference(const Event_<T> &e) const {
    return t - e.t;
  }

  /*!
  \brief Overload of << operator.
  \param os Output stream
  \param e Event to print
  \return Output stream
  */
  [[nodiscard]] friend std::ostream &operator<<(std::ostream &os, const Event_ &e) {
    os << std::string(" (" + std::to_string(e.x) + "," + std::to_string(e.y) + ") " + std::to_string(e.t) + (e.p ? " [+]" : " [-]"));
    return os;
  }
};
using Eventi = Event_<int>;    /*!< Alias for Event_ using int */
using Eventl = Event_<long>;   /*!< Alias for Event_ using long */
using Eventf = Event_<float>;  /*!< Alias for Event_ using float */
using Eventd = Event_<double>; /*!< Alias for Event_ using double */
using Event = Eventi;          /*!< Alias for Event_ using int */

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
class Size2_ : public cv::Size_<T> {
  using cv::Size_<T>::Size_;
};
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
  using cv::Size_<T>::Size_;

public:
  T length; /*!< Temporal dimension */

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
    return Size3_<T>::width && Size3_<T>::height && length;
  }

  /*!
  Volume is computed as \f$ w \cdot h \cdot l \f$.
  \brief Compute the volume.
  \return Volume
  */
  [[nodiscard]] inline T volume() const {
    return Size3_<T>::width * Size3_<T>::height * length;
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
class Rect2_ : public cv::Rect_<T> {
  using cv::Rect_<T>::Rect_;

public:
  /*!
  \brief Check if the rectangle contains an event.
  \param e Event to check
  \return True if the event is inside
  */
  [[nodiscard]] inline bool contains(const Event_<T> &e) const {
    return e.inside(*this);
  };
};
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
  Rect3_() : cv::Rect_<T>(){};

  /*! \cond INTERNAL */
  ~Rect3_() = default;
  /*! \endcond */

  /*!
  Copy constructor.
  */
  Rect3_(const Rect3_<T> &) = default;

  /*!
  Move constructor.
  */
  Rect3_(Rect3_<T> &&) noexcept = default;

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
  Copy assignment operator
  */
  Rect3_<T> &operator=(const Rect3_<T> &) = default;

  /*!
  Move assignment operator
  */
  Rect3_<T> &operator=(Rect3_<T> &&) noexcept = default;

  /*!
  \brief Check if empty.
  \return True if empty
  */
  [[nodiscard]] inline bool empty() const {
    return Rect3_<T>::width && Rect3_<T>::height && length;
  }

  /*!
  \brief Check if the rectangular cuboid contains an event.
  \param e Event to check
  \return True if the event is inside
  */
  [[nodiscard]] inline bool contains(const Event_<T> &e) const {
    return e.inside(*this) && e.t >= t && e.t <= t + length;
  };

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

/*! \cond INTERNAL */
class EventPacket;
class EventBuffer;
/*! \endcond */

/*!
\brief This class extends std::vector<Event> to implement event packets. For more information, please refer <a href="https://en.cppreference.com/w/cpp/container/vector">here</a>.

Event packets inherit all the properties from standard C++ vectors. Events in the packet are stored contiguously.
*/
class EventPacket : public std::vector<Event> {
  using std::vector<Event>::vector;

public:
  /*!
  \brief Insert elements from a buffer of events.
  \param eb Event buffer to insert
  */
  inline void insert(EventBuffer &eb);

  /*!
  \brief Time difference between the last and the first event in the packet.
  \return Time difference
  */
  [[nodiscard]] inline double duration() const {
    return (this->back()).t - (this->front()).t;
  }
};

/*!
\brief This class extends std::queue<Event> to implement event buffers. For more information, please refer <a href="https://en.cppreference.com/w/cpp/container/queue">here</a>.

Event buffers inherit all the properties from standard C++ queues. Events buffers are FIFO data structures not intended to be directly iterated.
*/
class EventBuffer : public std::queue<Event> {
  using std::queue<Event>::queue;

public:
  /*!
  \brief Insert elements from a packet of events.
  \param ep Event packet to insert
  */
  inline void insert(const EventPacket &ep);

  /*!
  \brief Time difference between the last and the first event in the buffer.
  \return Time difference
  */
  [[nodiscard]] inline double duration() const {
    return (this->back()).t - (this->front()).t;
  }
};

inline void EventPacket::insert(EventBuffer &eb) {
  reserve(size() + eb.size());
  while(!eb.empty()) {
    push_back(eb.front());
    eb.pop();
  }
}

inline void EventBuffer::insert(const EventPacket &ep) {
  for(const Event &e : ep) push(e);
}

} // namespace ev

#endif // OPENEV_TYPES_HPP
