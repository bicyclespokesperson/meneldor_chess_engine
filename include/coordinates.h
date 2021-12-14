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

    m_square_index = y * c_board_dimension + x;
  }

  explicit constexpr Coordinates(int32_t square_index)
  {
    MY_ASSERT(0 <= square_index && square_index < c_board_dimension_squared, "A board has 64 squares");

    m_square_index = square_index;
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

#endif
