#include "move_orderer.h"

int Move_orderer::score_move_(Move m, Board const& /* board */)
{
  auto const m_victim_index{static_cast<uint8_t>(m.victim()) - static_cast<uint8_t>(Piece::pawn)};
  auto const m_attacker_index{static_cast<uint8_t>(m.piece()) - static_cast<uint8_t>(Piece::pawn)};

  auto const score = mvv_lva_table[m_victim_index][m_attacker_index];

  return score;
}

const auto Move_orderer::mvv_lva_table = []
{
  static constexpr std::array pieces{Piece::king,   Piece::queen, Piece::rook, Piece::bishop,
                                     Piece::knight, Piece::pawn,  Piece::empty};

  //NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init) Initialized in this function
  std::array<std::array<int, c_piece_count>, c_piece_count> result;

  // Create a lookup table of move priorities. Move with the most valuable victim piece will be prioritied highest,
  // and moves with the same victim will be prioritized by the least valuable attacker. Higher priorities correspond
  // to better moves

  int current_val{1};

  // Start with the least valuable victim, and increment current_val as we go so
  // more valuable victims have higher priorities
  for (size_t victim{c_piece_count}; victim > 0; --victim)
  {
    for (size_t attacker{0}; attacker < c_piece_count; ++attacker)
    {
      auto victim_index = static_cast<uint8_t>(pieces[victim - 1]) - static_cast<uint8_t>(Piece::pawn);
      auto attacker_index = static_cast<uint8_t>(pieces[attacker]) - static_cast<uint8_t>(Piece::pawn);
      result[victim_index][attacker_index] = current_val;
      ++current_val;
    }
  }

  return result;
}();

void Move_orderer::sort_moves(std::span<Move> moves, Board const& board) const
{
  std::sort(moves.begin(), moves.end(),
            [&board](Move m1, Move m2)
            {
              return score_move_(m1, board) > score_move_(m2, board);
            });
}
