#ifndef COORDINATES_H
#define COORDINATES_H

#include "chess_types.h"
#include "my_assert.h"

class Coordinates
{
public:
  static std::optional<Coordinates> from_str(std::string_view str);

  constexpr Coordinates(int32_t x, int32_t y)
  {
    MY_ASSERT(0 <= x && x < 8, "A board has coordinates 0-7");
    MY_ASSERT(0 <= y && y < 8, "A board has coordinates 0-7");
    static_assert(sizeof(Coordinates) == 1);

    m_square_index = static_cast<uint8_t>(y * c_board_dimension + x);
  }

  explicit constexpr Coordinates(int32_t square_index)
  {
    MY_ASSERT(0 <= square_index && square_index < c_board_dimension_squared, "A board has 64 squares");

    m_square_index = static_cast<uint8_t>(square_index);
  }

  constexpr Coordinates(Coordinates const& other) = default;

  constexpr Coordinates& operator=(Coordinates const& other) = default;

  constexpr int32_t x() const
  {
    return m_square_index % c_board_dimension;
  }

  constexpr int32_t y() const
  {
    return m_square_index / c_board_dimension;
  }

  constexpr int32_t square_index() const
  {
    return m_square_index;
  }

  constexpr auto operator<=>(Coordinates const& other) const = default;

private:
  uint8_t m_square_index{0};
};

std::ostream& operator<<(std::ostream& os, Coordinates const& self);

constexpr Coordinates a1{0, 0};
constexpr Coordinates a2{0, 1};
constexpr Coordinates a3{0, 2};
constexpr Coordinates a4{0, 3};
constexpr Coordinates a5{0, 4};
constexpr Coordinates a6{0, 5};
constexpr Coordinates a7{0, 6};
constexpr Coordinates a8{0, 7};
constexpr Coordinates b1{1, 0};
constexpr Coordinates b2{1, 1};
constexpr Coordinates b3{1, 2};
constexpr Coordinates b4{1, 3};
constexpr Coordinates b5{1, 4};
constexpr Coordinates b6{1, 5};
constexpr Coordinates b7{1, 6};
constexpr Coordinates b8{1, 7};
constexpr Coordinates c1{2, 0};
constexpr Coordinates c2{2, 1};
constexpr Coordinates c3{2, 2};
constexpr Coordinates c4{2, 3};
constexpr Coordinates c5{2, 4};
constexpr Coordinates c6{2, 5};
constexpr Coordinates c7{2, 6};
constexpr Coordinates c8{2, 7};
constexpr Coordinates d1{3, 0};
constexpr Coordinates d2{3, 1};
constexpr Coordinates d3{3, 2};
constexpr Coordinates d4{3, 3};
constexpr Coordinates d5{3, 4};
constexpr Coordinates d6{3, 5};
constexpr Coordinates d7{3, 6};
constexpr Coordinates d8{3, 7};
constexpr Coordinates e1{4, 0};
constexpr Coordinates e2{4, 1};
constexpr Coordinates e3{4, 2};
constexpr Coordinates e4{4, 3};
constexpr Coordinates e5{4, 4};
constexpr Coordinates e6{4, 5};
constexpr Coordinates e7{4, 6};
constexpr Coordinates e8{4, 7};
constexpr Coordinates f1{5, 0};
constexpr Coordinates f2{5, 1};
constexpr Coordinates f3{5, 2};
constexpr Coordinates f4{5, 3};
constexpr Coordinates f5{5, 4};
constexpr Coordinates f6{5, 5};
constexpr Coordinates f7{5, 6};
constexpr Coordinates f8{5, 7};
constexpr Coordinates g1{6, 0};
constexpr Coordinates g2{6, 1};
constexpr Coordinates g3{6, 2};
constexpr Coordinates g4{6, 3};
constexpr Coordinates g5{6, 4};
constexpr Coordinates g6{6, 5};
constexpr Coordinates g7{6, 6};
constexpr Coordinates g8{6, 7};
constexpr Coordinates h1{7, 0};
constexpr Coordinates h2{7, 1};
constexpr Coordinates h3{7, 2};
constexpr Coordinates h4{7, 3};
constexpr Coordinates h5{7, 4};
constexpr Coordinates h6{7, 5};
constexpr Coordinates h7{7, 6};
constexpr Coordinates h8{7, 7};

#endif
