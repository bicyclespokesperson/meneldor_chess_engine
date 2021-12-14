#ifndef MOVE_ORDERER_H
#define MOVE_ORDERER_H

#include "chess_types.h"
#include "move.h"

class Board;

class Move_orderer
{
public:
  Move_orderer() = default;
  ~Move_orderer() = default;

  void sort_moves(std::span<Move> moves, Board const& board) const;

private:
  static int score_move_(Move m, Board const& board);

  static constexpr size_t c_piece_count = static_cast<uint8_t>(Piece::_count) - static_cast<uint8_t>(Piece::pawn);
  static const std::array<std::array<int, c_piece_count>, c_piece_count> mvv_lva_table;
};

#endif // MOVE_ORDERER_H
