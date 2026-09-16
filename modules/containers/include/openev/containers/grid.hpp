/*!
\file grid.hpp
\brief Grid of event containers for basic event structures.
\author Raul Tapia
*/
#ifndef OPENEV_CONTAINERS_GRID_HPP
#define OPENEV_CONTAINERS_GRID_HPP

#include "openev/core/types.hpp"
#include <cstddef>
#include <opencv2/core/base.hpp>
#include <opencv2/core/types.hpp>
#include <type_traits>
#include <utility>
#include <vector>

namespace ev {
[[maybe_unused]] constexpr bool USING_GRID_HPP = true;

/*! \cond INTERNAL */
template <typename Container, typename = void>
struct has_push_back_ : std::false_type {};
template <typename Container>
struct has_push_back_<Container, std::void_t<decltype(std::declval<Container &>().push_back(std::declval<const typename Container::value_type &>()))>> : std::true_type {};

template <typename Container, typename = void>
struct has_push_ : std::false_type {};
template <typename Container>
struct has_push_<Container, std::void_t<decltype(std::declval<Container &>().push(std::declval<const typename Container::value_type &>()))>> : std::true_type {};

template <typename Container, typename = void>
struct has_clear_ : std::false_type {};
template <typename Container>
struct has_clear_<Container, std::void_t<decltype(std::declval<Container &>().clear())>> : std::true_type {};
/*! \endcond */

/*!
\brief This class arranges any event container into a grid of MxN cells covering the sensor.

Any container that offers push() or push_back() can be used as cell; push() takes precedence, so containers such as
ev::SlidingWindow_ keep their own insertion policy:
\code{.cpp}
ev::Grid_<ev::Vector> grid(cv::Size(640, 480), cv::Size(4, 3));
ev::Grid_<ev::CircularBuffer> grid(cv::Size(640, 480), cv::Size(4, 3), 100);
ev::Grid_<ev::SlidingWindow> grid(cv::Size(640, 480), cv::Size(4, 3), 0.1);
\endcode
*/
template <typename Container>
class Grid_ {
  static_assert(has_push_back_<Container>::value || has_push_<Container>::value, "ev::Grid_: the container must offer push_back() or push().");

public:
  using EventType = typename Container::value_type; /*!< Event type held by the cells */
  using T = typename EventType::value_type;         /*!< Coordinate type of the events */

  /*!
  \brief Construct a grid of cells covering the sensor.
  \param sensor Sensor size in pixels
  \param cells Number of cells: width columns and height rows
  \param args Arguments forwarded to the constructor of every cell
  */
  template <typename... Args>
  explicit Grid_(const cv::Size sensor, const cv::Size cells, Args &&...args) : sensor_{sensor}, shape_{cells}, prototype_(std::forward<Args>(args)...) {
    if(sensor.width <= 0 || sensor.height <= 0) {
      CV_Error(cv::Error::StsBadArg, "ev::Grid_: the sensor size must be positive.");
    }
    if(cells.width <= 0 || cells.height <= 0 || cells.width > sensor.width || cells.height > sensor.height) {
      CV_Error(cv::Error::StsBadArg, "ev::Grid_: the number of cells must be positive and not exceed the sensor size.");
    }
    cell_ = cv::Size((sensor.width + cells.width - 1) / cells.width, (sensor.height + cells.height - 1) / cells.height);
    cells_.assign(static_cast<std::size_t>(cells.width) * static_cast<std::size_t>(cells.height), prototype_);
  }

  /*!
  \brief Insert an event into the cell its coordinates fall into.
  \param e Event to insert
  \return True if the event is inside the sensor and was inserted
  */
  inline bool insert(const EventType &e) {
    const cv::Point c = cell(e);
    if(c.x < 0) {
      return false;
    }
    push_(cells_[index_(c)], e);
    return true;
  }

  /*!
  \brief Cell an event falls into.
  \param e Event
  \return Cell as (column, row), or (-1, -1) if the event is outside the sensor
  */
  [[nodiscard]] inline cv::Point cell(const EventType &e) const {
    int x = 0;
    int y = 0;
    if constexpr(std::is_floating_point_v<T>) {
      x = round_(e.x);
      y = round_(e.y);
    } else {
      x = static_cast<int>(e.x);
      y = static_cast<int>(e.y);
    }
    if(x < 0 || y < 0 || x >= sensor_.width || y >= sensor_.height) {
      return {-1, -1};
    }
    return {x / cell_.width, y / cell_.height};
  }

  /*!
  \brief Access a cell.
  \param row Row index
  \param col Column index
  \return Reference to the cell container
  */
  [[nodiscard]] inline Container &operator()(const int row, const int col) {
    return cells_[index_({col, row})];
  }

  /*!
  \brief Access a cell.
  \param row Row index
  \param col Column index
  \return Const reference to the cell container
  */
  [[nodiscard]] inline const Container &operator()(const int row, const int col) const {
    return cells_[index_({col, row})];
  }

  /*!
  \brief Access a cell.
  \param c Cell as (column, row)
  \return Reference to the cell container
  */
  [[nodiscard]] inline Container &operator()(const cv::Point c) {
    return cells_[index_(c)];
  }

  /*!
  \brief Access a cell.
  \param c Cell as (column, row)
  \return Const reference to the cell container
  */
  [[nodiscard]] inline const Container &operator()(const cv::Point c) const {
    return cells_[index_(c)];
  }

  /*!
  \brief Empty every cell.
  */
  inline void clear() {
    if constexpr(has_clear_<Container>::value) {
      for(Container &cell : cells_) {
        cell.clear();
      }
    } else {
      cells_.assign(cells_.size(), prototype_);
    }
  }

  /*!
  \brief Number of rows of the grid.
  \return Rows
  */
  [[nodiscard]] inline int rows() const {
    return shape_.height;
  }

  /*!
  \brief Number of columns of the grid.
  \return Columns
  */
  [[nodiscard]] inline int cols() const {
    return shape_.width;
  }

  /*!
  \brief Number of cells: width columns and height rows.
  \return Grid shape
  */
  [[nodiscard]] inline cv::Size size() const {
    return shape_;
  }

  /*!
  \brief Size of each cell in pixels.
  \return Cell size
  */
  [[nodiscard]] inline cv::Size cellSize() const {
    return cell_;
  }

  /*!
  \brief Size of the sensor in pixels.
  \return Sensor size
  */
  [[nodiscard]] inline cv::Size sensorSize() const {
    return sensor_;
  }

  /*! \cond INTERNAL */
  [[nodiscard]] inline auto begin() {
    return cells_.begin();
  }

  [[nodiscard]] inline auto end() {
    return cells_.end();
  }

  [[nodiscard]] inline auto begin() const {
    return cells_.begin();
  }

  [[nodiscard]] inline auto end() const {
    return cells_.end();
  }
  /*! \endcond */

private:
  cv::Size sensor_;
  cv::Size shape_;
  cv::Size cell_;
  Container prototype_;
  std::vector<Container> cells_;

  [[nodiscard]] inline std::size_t index_(const cv::Point c) const {
    return (static_cast<std::size_t>(c.y) * static_cast<std::size_t>(shape_.width)) + static_cast<std::size_t>(c.x);
  }

  static inline void push_(Container &container, const EventType &e) {
    if constexpr(has_push_<Container>::value) {
      container.push(e);
    } else {
      container.push_back(e);
    }
  }
};
} // namespace ev

#endif // OPENEV_CONTAINERS_GRID_HPP
