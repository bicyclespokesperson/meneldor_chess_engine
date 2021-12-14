#ifndef MOVE_GENERATOR_H
#define MOVE_GENERATOR_H

#include "bitboard.h"
#include "chess_types.h"
#include "coordinates.h"
#include "move.h"

class Board;

class Move_generator
{
public:
  static uint64_t perft(int depth, Board& board, std::atomic_flag& is_cancelled);

  static std::vector<Move> generate_legal_moves(Board const& board);
  static std::vector<Move> generate_legal_attack_moves(Board const& board);
  static std::vector<Move> generate_pseudo_legal_attack_moves(Board const& board);
  static std::vector<Move> generate_pseudo_legal_moves(Board const& board);
  static bool has_any_legal_moves(Board const& board);
  static Bitboard get_all_attacked_squares(Board const& board, Color attacking_color);
  static bool is_square_attacked(Board const& board, Color attacking_color, Bitboard attacked_square);

public:
  class Tables
  {
  public:
    Tables();

    // Attacked squares from each position for a knight
    std::array<Bitboard, c_board_dimension_squared> knight_attacks;

    // Attacked squares from each position for a king
    std::array<Bitboard, c_board_dimension_squared> king_attacks;

    // What squares are able to block a piece on each square
    std::array<Bitboard, 64> bishop_possible_blockers;
    std::array<Bitboard, 64> rook_possible_blockers;

    std::array<int, 64> bishop_attacks_starting_offset{};
    std::array<int, 64> rook_attacks_starting_offset{};

    // Table of possible attacked squares for a rook & bishop for each possible set of blockers
    constexpr static size_t c_attacks_table_size{107'648};
    std::array<Bitboard, c_attacks_table_size> attacks;

  private:
    int init_bishop_magic_tables_(int index, int start_offset);
    int init_rook_magic_tables_(int index, int start_offset);

    void initialize_knight_attacks_();
    void initialize_king_attacks_();
  };

  static const Tables m_tables;
};

struct Bitboard_constants
{
  static constexpr Bitboard first_rank{0x00000000000000ff};
  static constexpr Bitboard second_rank{0x000000000000ff00};
  static constexpr Bitboard third_rank{0x0000000000ff0000};
  static constexpr Bitboard forth_rank{0x00000000ff000000};
  static constexpr Bitboard fifth_rank{0x000000ff00000000};
  static constexpr Bitboard sixth_rank{0x0000ff0000000000};
  static constexpr Bitboard seventh_rank{0x00ff000000000000};
  static constexpr Bitboard eighth_rank{0xff00000000000000};
  static constexpr Bitboard a_file{0x0101010101010101};
  static constexpr Bitboard h_file{0x8080808080808080};
  static constexpr Bitboard all_outer_squares{0xff818181818181ff};
  static constexpr Bitboard corners{0x8100000000000081};

  static constexpr Bitboard short_castling_empty_squares_white{0x0000000000000060};
  static constexpr Bitboard long_castling_empty_squares_white{0x000000000000000c};
  static constexpr Bitboard short_castling_empty_squares_black{0x6000000000000000};
  static constexpr Bitboard long_castling_empty_squares_black{0x0c00000000000000};

  static constexpr Bitboard all{std::numeric_limits<uint64_t>::max()};
  static constexpr Bitboard none{0};
};

#endif // MOVE_GENERATOR_H
