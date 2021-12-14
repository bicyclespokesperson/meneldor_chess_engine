#include "zobrist_hash.h"
#include "board.h"

// https://www.chessprogramming.org/Zobrist_Hashing

namespace
{

} // namespace

Zobrist_hash::Zobrist_hash(Board const& board)
{
  for (auto piece : piece_types)
  {
    auto const piece_offset = static_cast<uint8_t>(piece) - static_cast<uint8_t>(Piece::pawn);
    for (auto const sq_index : board.get_piece_set(Color::black, piece))
    {
      auto const rand_index = c_black_pieces_offset + (c_board_dimension_squared * piece_offset) + sq_index;
      m_hash ^= m_random_numbers[rand_index];
    }
    for (auto const sq_index : board.get_piece_set(Color::white, piece))
    {
      auto const rand_index = c_white_pieces_offset + (c_board_dimension_squared * piece_offset) + sq_index;
      m_hash ^= m_random_numbers[rand_index];
    }
  }

  if (board.get_active_color() == Color::black)
  {
    update_player_to_move();
  }

  update_en_passant_square(board.get_en_passant_square());
  update_castling_rights(board.get_castling_rights());
}

std::ostream& operator<<(std::ostream& os, Zobrist_hash const& self)
{
  os << std::to_string(self.get_hash());
  return os;
}
