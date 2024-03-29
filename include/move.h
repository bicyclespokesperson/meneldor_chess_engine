#ifndef MOVE_H
#define MOVE_H

#include "chess_types.h"
#include "coordinates.h"

namespace Meneldor
{
struct Move
{
  constexpr Move()
  {
    set_from(a1);
    set_to(a1);
    set_piece(Piece::black);
    set_victim(Piece::black);
    set_promotion(Piece::black);
    set_type(Move_type::null);
    set_score(0);

    MY_ASSERT(m_val == 0, "Move should be initialized to zero by default");
  }

  // Victim should be Piece::empty if nothing was captured
  constexpr Move(Coordinates from_coord,
                 Coordinates to_coord,
                 Piece moving_piece,
                 Piece victim_piece,
                 Piece promotion_piece = Piece::empty,
                 Move_type move_type = Move_type::normal,
                 uint8_t score = 0)
  {
    MY_ASSERT(!(static_cast<uint8_t>(moving_piece) & 0xf0),
              "Ensure we aren't throwing away any information about the piece");
    MY_ASSERT(!(static_cast<uint8_t>(victim_piece) & 0xf0),
              "Ensure we aren't throwing away any information about the piece");
    MY_ASSERT(!(static_cast<uint8_t>(promotion_piece) & 0xf0),
              "Ensure we aren't throwing away any information about the piece");

    m_val = 0;
    set_from(from_coord);
    set_to(to_coord);
    set_piece(moving_piece);
    set_victim(victim_piece);
    set_promotion(promotion_piece);
    set_type(move_type);
    set_score(score);
  }

  constexpr Coordinates from() const
  {
    return Coordinates{static_cast<int32_t>(m_val & 0x0000003f)};
  }

  constexpr void set_from(Coordinates from)
  {
    MY_ASSERT(from.square_index() < c_board_dimension_squared, "Square index too large");
    m_val |= static_cast<uint8_t>(from.square_index());
    MY_ASSERT(this->from() == from, "Invalid state");
  }

  constexpr Coordinates to() const
  {
    return Coordinates{static_cast<int32_t>((m_val & 0x00000fc0) >> 6)};
  }

  constexpr void set_to(Coordinates to)
  {
    MY_ASSERT(this->from().square_index() < c_board_dimension_squared, "Square index too large");
    m_val |= (static_cast<uint8_t>(to.square_index()) << 6);
    MY_ASSERT(this->to() == to, "Invalid state");
  }

  constexpr Piece piece() const
  {
    return static_cast<Piece>((m_val & 0x0000f000) >> 12);
  }

  constexpr void set_piece(Piece piece)
  {
    m_val |= (static_cast<uint8_t>(piece) << 12);
    MY_ASSERT(this->piece() == piece, "Invalid state");
  }

  constexpr Piece victim() const
  {
    return static_cast<Piece>((m_val & 0x000f0000) >> 16);
  }

  constexpr void set_victim(Piece victim)
  {
    m_val |= (static_cast<uint8_t>(victim) << 16);
    MY_ASSERT(this->victim() == victim, "Invalid state");
  }

  constexpr Piece promotion() const
  {
    return static_cast<Piece>((m_val & 0x00f00000) >> 20);
  }

  constexpr void set_promotion(Piece promotion)
  {
    m_val |= (static_cast<uint8_t>(promotion) << 20);
    MY_ASSERT(this->promotion() == promotion, "Invalid state");
  }

  constexpr Move_type type() const
  {
    return static_cast<Move_type>((m_val & 0x0f000000) >> 24);
  }

  constexpr void set_type(Move_type type)
  {
    m_val |= (static_cast<uint8_t>(type) << 24);
    MY_ASSERT(this->type() == type, "Invalid state");
  }

  constexpr uint8_t score() const
  {
    return uint8_t{0x0f} & static_cast<uint8_t>((m_val & 0xf0000000) >> 28);
  }

  constexpr void set_score(uint8_t score)
  {
    m_val &= ~0xf0000000; // Clear score field
    m_val |= (static_cast<uint8_t>(score) << 28);
    MY_ASSERT(this->score() == score, "Invalid state");
  }

  constexpr auto operator<=>(Move const& other) const = default;

#if 0
  // Layout:
  Coordinates from : 6;
  Coordinates to : 6;
  Piece piece : 4 {Piece::empty};

  // Piece::empty represents no piece will be captured by this move
  Piece victim : 4 {Piece::empty};
  Piece promotion : 4 {Piece::empty};
  Move_type type : 4 {Move_type::null};
  uint8_t score : 4 {0};
#endif

private:
  uint32_t m_val{0};
};

static_assert(sizeof(Move) == 4);

std::ostream& operator<<(std::ostream& os, Move const& self);
} // namespace Meneldor

#endif // MOVE_H
