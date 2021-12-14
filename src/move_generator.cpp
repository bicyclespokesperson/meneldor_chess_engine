#include "move_generator.h"
#include "board.h"
#include "feature_toggle.h"
#include "my_assert.h"

namespace
{

// Generated with code from: https://www.chessprogramming.org/Looking_for_Magics
constexpr std::array<uint64_t, c_board_dimension_squared> c_rook_magics{
  0xa8002c000108020ULL,  0x6c00049b0002001ULL,  0x100200010090040ULL,  0x2480041000800801ULL, 0x280028004000800ULL,
  0x900410008040022ULL,  0x280020001001080ULL,  0x2880002041000080ULL, 0xa000800080400034ULL, 0x4808020004000ULL,
  0x2290802004801000ULL, 0x411000d00100020ULL,  0x402800800040080ULL,  0xb000401004208ULL,    0x2409000100040200ULL,
  0x1002100004082ULL,    0x22878001e24000ULL,   0x1090810021004010ULL, 0x801030040200012ULL,  0x500808008001000ULL,
  0xa08018014000880ULL,  0x8000808004000200ULL, 0x201008080010200ULL,  0x801020000441091ULL,  0x800080204005ULL,
  0x1040200040100048ULL, 0x120200402082ULL,     0xd14880480100080ULL,  0x12040280080080ULL,   0x100040080020080ULL,
  0x9020010080800200ULL, 0x813241200148449ULL,  0x491604001800080ULL,  0x100401000402001ULL,  0x4820010021001040ULL,
  0x400402202000812ULL,  0x209009005000802ULL,  0x810800601800400ULL,  0x4301083214000150ULL, 0x204026458e001401ULL,
  0x40204000808000ULL,   0x8001008040010020ULL, 0x8410820820420010ULL, 0x1003001000090020ULL, 0x804040008008080ULL,
  0x12000810020004ULL,   0x1000100200040208ULL, 0x430000a044020001ULL, 0x280009023410300ULL,  0xe0100040002240ULL,
  0x200100401700ULL,     0x2244100408008080ULL, 0x8000400801980ULL,    0x2000810040200ULL,    0x8010100228810400ULL,
  0x2000009044210200ULL, 0x4080008040102101ULL, 0x40002080411d01ULL,   0x2005524060000901ULL, 0x502001008400422ULL,
  0x489a000810200402ULL, 0x1004400080a13ULL,    0x4000011008020084ULL, 0x26002114058042ULL,
};

constexpr std::array<uint64_t, c_board_dimension_squared> c_bishop_magics{
  0x89a1121896040240ULL, 0x2004844802002010ULL, 0x2068080051921000ULL, 0x62880a0220200808ULL, 0x4042004000000ULL,
  0x100822020200011ULL,  0xc00444222012000aULL, 0x28808801216001ULL,   0x400492088408100ULL,  0x201c401040c0084ULL,
  0x840800910a0010ULL,   0x82080240060ULL,      0x2000840504006000ULL, 0x30010c4108405004ULL, 0x1008005410080802ULL,
  0x8144042209100900ULL, 0x208081020014400ULL,  0x4800201208ca00ULL,   0xf18140408012008ULL,  0x1004002802102001ULL,
  0x841000820080811ULL,  0x40200200a42008ULL,   0x800054042000ULL,     0x88010400410c9000ULL, 0x520040470104290ULL,
  0x1004040051500081ULL, 0x2002081833080021ULL, 0x400c00c010142ULL,    0x941408200c002000ULL, 0x658810000806011ULL,
  0x188071040440a00ULL,  0x4800404002011c00ULL, 0x104442040404200ULL,  0x511080202091021ULL,  0x4022401120400ULL,
  0x80c0040400080120ULL, 0x8040010040820802ULL, 0x480810700020090ULL,  0x102008e00040242ULL,  0x809005202050100ULL,
  0x8002024220104080ULL, 0x431008804142000ULL,  0x19001802081400ULL,   0x200014208040080ULL,  0x3308082008200100ULL,
  0x41010500040c020ULL,  0x4012020c04210308ULL, 0x208220a202004080ULL, 0x111040120082000ULL,  0x6803040141280a00ULL,
  0x2101004202410000ULL, 0x8200000041108022ULL, 0x21082088000ULL,      0x2410204010040ULL,    0x40100400809000ULL,
  0x822088220820214ULL,  0x40808090012004ULL,   0x910224040218c9ULL,   0x402814422015008ULL,  0x90014004842410ULL,
  0x1000042304105ULL,    0x10008830412a00ULL,   0x2520081090008908ULL, 0x40102000a0a60140ULL,
};

// Occupancy of (blockers & rook attacks) for each square
constexpr std::array<uint64_t, c_board_dimension_squared> RBits{
  12, 11, 11, 11, 11, 11, 11, 12, 11, 10, 10, 10, 10, 10, 10, 11, 11, 10, 10, 10, 10, 10,
  10, 11, 11, 10, 10, 10, 10, 10, 10, 11, 11, 10, 10, 10, 10, 10, 10, 11, 11, 10, 10, 10,
  10, 10, 10, 11, 11, 10, 10, 10, 10, 10, 10, 11, 12, 11, 11, 11, 11, 11, 11, 12};

constexpr std::array<uint64_t, c_board_dimension_squared> BBits{
  6, 5, 5, 5, 5, 5, 5, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 7, 7, 7, 7, 5, 5, 5, 5, 7, 9, 9, 7, 5, 5,
  5, 5, 7, 9, 9, 7, 5, 5, 5, 5, 7, 7, 7, 7, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 5, 5, 5, 5, 5, 5, 6};

constexpr void update_if_in_bounds_(Bitboard& bb, int x, int y)
{
  if (0 <= x && x < c_board_dimension && 0 <= y && y < c_board_dimension)
  {
    bb.set_square(Coordinates{x, y}.square_index());
  }
}

// The following functions rely on colors casting to these values
static_assert(static_cast<uint8_t>(Color::black) == 0);
static_assert(static_cast<uint8_t>(Color::white) == 1);

using shift_fn = Bitboard (Bitboard::*)(int32_t) const;

// This array can be indexed by Color, so the operator to be used for black pawns is first
constexpr std::array c_shift_functions{&Bitboard::operator>>, &Bitboard::operator<< };
constexpr shift_fn get_pawn_shift_fn(Color color)
{
  return c_shift_functions[static_cast<int32_t>(color)];
}

// This array can be indexed by Color, so the operator to be used for black pawns is first
constexpr std::array c_start_ranks{Bitboard_constants::seventh_rank, Bitboard_constants::second_rank};
constexpr Bitboard get_pawn_start_rank(Color color)
{
  return c_start_ranks[static_cast<int32_t>(color)];
}

constexpr std::array c_square_offsets{c_board_dimension, -c_board_dimension};
constexpr int32_t get_start_square_offset(Color color)
{
  return c_square_offsets[static_cast<int32_t>(color)];
}

constexpr std::array c_east_shift_offsets{7, 9};
constexpr int32_t get_east_shift_distance(Color color)
{
  return c_east_shift_offsets[static_cast<int32_t>(color)];
}

constexpr std::array c_west_shift_offsets{9, 7};
constexpr int32_t get_west_shift_distance(Color color)
{
  return c_west_shift_offsets[static_cast<int32_t>(color)];
}

constexpr std::array c_east_offsets{7, -9};
constexpr int32_t get_east_capture_offset(Color color)
{
  return c_east_offsets[static_cast<int32_t>(color)];
}

constexpr std::array c_west_offsets{9, -7};
constexpr int32_t get_west_capture_offset(Color color)
{
  return c_west_offsets[static_cast<int32_t>(color)];
}

int pop_1st_bit(uint64_t* bb)
{
  constexpr static std::array<int, 64> BitTable{63, 30, 3,  32, 25, 41, 22, 33, 15, 50, 42, 13, 11, 53, 19, 34,
                                                61, 29, 2,  51, 21, 43, 45, 10, 18, 47, 1,  54, 9,  57, 0,  35,
                                                62, 31, 40, 4,  49, 5,  52, 26, 60, 6,  23, 44, 46, 27, 56, 16,
                                                7,  39, 48, 24, 59, 14, 12, 55, 38, 28, 58, 20, 37, 17, 36, 8};

  uint64_t b = *bb ^ (*bb - 1);
  auto fold = static_cast<uint32_t>((b & 0xffffffff) ^ (b >> 32));
  *bb &= (*bb - 1);
  return BitTable[(fold * 0x783a9b23) >> 26];
}

// Given a bitboard with n permutations (x one bits -> 2^x permutations), returns
// the index'th permutation for the bitboard. Must be called n times to generate
// every permutation.
uint64_t blocker_permutation_from_index(int index, int bits, uint64_t m)
{
  uint64_t result = 0ULL;
  for (int i = 0; i < bits; i++)
  {
    int j = pop_1st_bit(&m);
    if (index & (1 << i))
    {
      result |= (1ULL << j);
    }
  }
  return result;
}

// Returns the possible blockers mask for a rook on the given square
// Outer squares are not possible blockers, unless the rook is on an outer
// square. In that case, other pieces on the same row or column are potential
// blockers.
Bitboard rook_potential_blockers(int sq)
{
  Bitboard result{0ULL};
  int rank = sq / 8;
  int file = sq % 8;
  for (int r = rank + 1; r <= 6; r++)
  {
    result |= (Bitboard{1ULL} << (file + r * 8));
  }
  for (int r = rank - 1; r >= 1; r--)
  {
    result |= (Bitboard{1ULL} << (file + r * 8));
  }
  for (int f = file + 1; f <= 6; f++)
  {
    result |= (Bitboard{1ULL} << (f + rank * 8));
  }
  for (int f = file - 1; f >= 1; f--)
  {
    result |= (Bitboard{1ULL} << (f + rank * 8));
  }
  return result;
}

// Returns the possible blockers mask for a bishop on the given square
// Outer squares are not possible blockers for a bishop
Bitboard bishop_potential_blockers(int sq)
{
  Bitboard result{0ULL};
  int rank = sq / 8;
  int file = sq % 8;
  for (int r = rank + 1, f = file + 1; r <= 6 && f <= 6; r++, f++)
  {
    result |= (Bitboard{1ULL} << (f + r * 8));
  }
  for (int r = rank + 1, f = file - 1; r <= 6 && f >= 1; r++, f--)
  {
    result |= (Bitboard{1ULL} << (f + r * 8));
  }
  for (int r = rank - 1, f = file + 1; r >= 1 && f <= 6; r--, f++)
  {
    result |= (Bitboard{1ULL} << (f + r * 8));
  }
  for (int r = rank - 1, f = file - 1; r >= 1 && f >= 1; r--, f--)
  {
    result |= (Bitboard{1ULL} << (f + r * 8));
  }
  return result;
}

Bitboard rook_attacked_squares(int sq, uint64_t block)
{
  Bitboard result{0ULL};
  int rk = sq / 8, fl = sq % 8;
  for (int r = rk + 1; r <= 7; r++)
  {
    result |= (Bitboard{1ULL} << (fl + r * 8));
    if (block & (1ULL << (fl + r * 8)))
    {
      break;
    }
  }
  for (int r = rk - 1; r >= 0; r--)
  {
    result |= (Bitboard{1ULL} << (fl + r * 8));
    if (block & (1ULL << (fl + r * 8)))
    {
      break;
    }
  }
  for (int f = fl + 1; f <= 7; f++)
  {
    result |= (Bitboard{1ULL} << (f + rk * 8));
    if (block & (1ULL << (f + rk * 8)))
    {
      break;
    }
  }
  for (int f = fl - 1; f >= 0; f--)
  {
    result |= (Bitboard{1ULL} << (f + rk * 8));
    if (block & (1ULL << (f + rk * 8)))
    {
      break;
    }
  }
  return result;
}

Bitboard bishop_attacked_squares(int sq, uint64_t block)
{
  Bitboard result{0ULL};
  int rk = sq / 8, fl = sq % 8;
  for (int r = rk + 1, f = fl + 1; r <= 7 && f <= 7; r++, f++)
  {
    result |= (Bitboard{1ULL} << (f + r * 8));
    if (block & (1ULL << (f + r * 8)))
    {
      break;
    }
  }
  for (int r = rk + 1, f = fl - 1; r <= 7 && f >= 0; r++, f--)
  {
    result |= (Bitboard{1ULL} << (f + r * 8));
    if (block & (1ULL << (f + r * 8)))
    {
      break;
    }
  }
  for (int r = rk - 1, f = fl + 1; r >= 0 && f <= 7; r--, f++)
  {
    result |= (Bitboard{1ULL} << (f + r * 8));
    if (block & (1ULL << (f + r * 8)))
    {
      break;
    }
  }
  for (int r = rk - 1, f = fl - 1; r >= 0 && f >= 0; r--, f--)
  {
    result |= (Bitboard{1ULL} << (f + r * 8));
    if (block & (1ULL << (f + r * 8)))
    {
      break;
    }
  }
  return result;
}

int magic_hash_fn(uint64_t blockers, uint64_t magic, int bits)
{
  return static_cast<int>((blockers * magic) >> (64 - bits));
}

} // namespace

Move_generator::Tables const Move_generator::m_tables{};

Move_generator::Tables::Tables()
{
  initialize_knight_attacks_();
  initialize_king_attacks_();
  int start{0};
  for (int sq{0}; sq < c_board_dimension_squared; ++sq)
  {
    start = init_bishop_magic_tables_(sq, start);
    start = init_rook_magic_tables_(sq, start);
  }
}

int Move_generator::Tables::init_bishop_magic_tables_(int index, int starting_offset)
{
  // Based on code from: https://www.chessprogramming.org/Looking_for_Magics
  // (The plain implementation)

  Bitboard possible_blockers = bishop_potential_blockers(index);
  bishop_possible_blockers[index] = possible_blockers;

  // Populate blockers table
  int n = possible_blockers.occupancy();
  auto blocker_permutations = (1 << n);

  bishop_attacks_starting_offset[index] = starting_offset;
  for (int i = 0; i < blocker_permutations; ++i)
  {
    auto blockers = blocker_permutation_from_index(i, n, possible_blockers.val);

    int shift = BBits[index];
    auto key = magic_hash_fn(blockers, c_bishop_magics[index], shift);
    auto attacked_squares = bishop_attacked_squares(index, blockers);
    attacks[bishop_attacks_starting_offset[index] + key] = attacked_squares;
    ++starting_offset;
  }

  return starting_offset;
}

int Move_generator::Tables::init_rook_magic_tables_(int index, int starting_offset)
{
  // Based on code from: https://www.chessprogramming.org/Looking_for_Magics
  // (The "fancy" implementation)

  Bitboard possible_blockers = rook_potential_blockers(index);
  rook_possible_blockers[index] = possible_blockers;

  // Populate blockers table
  int n = possible_blockers.occupancy();
  auto blocker_permutations = (1 << n);

  rook_attacks_starting_offset[index] = starting_offset;
  for (int i = 0; i < blocker_permutations; ++i)
  {
    auto blockers = blocker_permutation_from_index(i, n, possible_blockers.val);

    int shift = RBits[index];
    auto key = magic_hash_fn(blockers, c_rook_magics[index], shift);
    auto attacked_squares = rook_attacked_squares(index, blockers);
    attacks[rook_attacks_starting_offset[index] + key] = attacked_squares;
    ++starting_offset;
  }

  return starting_offset;
}

void Move_generator::Tables::initialize_knight_attacks_()
{
  for (int8_t x{0}; x < c_board_dimension; ++x)
  {
    for (int8_t y{0}; y < c_board_dimension; ++y)
    {
      auto square_index = Coordinates{x, y}.square_index();
      auto& bb = knight_attacks[square_index];

      update_if_in_bounds_(bb, x + 1, y + 2);
      update_if_in_bounds_(bb, x - 1, y + 2);
      update_if_in_bounds_(bb, x + 1, y - 2);
      update_if_in_bounds_(bb, x - 1, y - 2);
      update_if_in_bounds_(bb, x + 2, y + 1);
      update_if_in_bounds_(bb, x - 2, y + 1);
      update_if_in_bounds_(bb, x + 2, y - 1);
      update_if_in_bounds_(bb, x - 2, y - 1);
    }
  }
}

void Move_generator::Tables::initialize_king_attacks_()
{
  for (int8_t x{0}; x < c_board_dimension; ++x)
  {
    for (int8_t y{0}; y < c_board_dimension; ++y)
    {
      auto square_index = Coordinates{x, y}.square_index();
      auto& bb = king_attacks[square_index];

      for (int i = -1; i <= 1; ++i)
      {
        for (int j = -1; j <= 1; ++j)
        {
          if (i != 0 || j != 0)
          {
            update_if_in_bounds_(bb, x + i, y + j);
          }
        }
      }
    }
  }
}

constexpr Bitboard rook_attacks(Coordinates square, Bitboard occupied)
{
  int const index = square.square_index();
  occupied &= Move_generator::m_tables.rook_possible_blockers[index];
  auto key = magic_hash_fn(occupied.val, c_rook_magics[index], RBits[index]);
  return Move_generator::m_tables.attacks[Move_generator::m_tables.rook_attacks_starting_offset[index] + key];
}

constexpr Bitboard bishop_attacks(Coordinates square, Bitboard occupied)
{
  int index = square.square_index();
  occupied &= Move_generator::m_tables.bishop_possible_blockers[index];
  auto key = magic_hash_fn(occupied.val, c_bishop_magics[index], BBits[index]);
  return Move_generator::m_tables.attacks[Move_generator::m_tables.bishop_attacks_starting_offset[index] + key];
}

constexpr Bitboard queen_attacks(Coordinates square, Bitboard occupied)
{
  return bishop_attacks(square, occupied) | rook_attacks(square, occupied);
}

constexpr Bitboard knight_attacks(Coordinates square, Bitboard /* occupied */)
{
  return Move_generator::m_tables.knight_attacks[square.square_index()];
}

constexpr Bitboard king_attacks(Coordinates square, Bitboard /* occupied */)
{
  return Move_generator::m_tables.king_attacks[square.square_index()];
}

template <Color color>
constexpr Bitboard pawn_short_advances(Bitboard pawns, Bitboard occupied)
{
  constexpr auto bitshift_fn = get_pawn_shift_fn(color);
  auto const one_space_moves = (pawns.*bitshift_fn)(c_board_dimension) & ~occupied;
  return one_space_moves;
}

template <Color color>
constexpr Bitboard pawn_long_advances(Bitboard pawns, Bitboard occupied)
{
  constexpr auto bitshift_fn = get_pawn_shift_fn(color);
  constexpr auto start_rank = get_pawn_start_rank(color);
  auto const eligible_pawns = pawns & start_rank;
  auto const one_space_moves = (eligible_pawns.*bitshift_fn)(c_board_dimension) & ~occupied;
  auto const two_space_moves = (one_space_moves.*bitshift_fn)(c_board_dimension) & ~occupied;
  return two_space_moves;
}

template <Color color>
constexpr Bitboard pawn_potential_attacks(Bitboard pawns)
{
  constexpr auto bitshift_fn = get_pawn_shift_fn(color);
  constexpr auto shift_distance_east = get_east_shift_distance(color);
  constexpr auto shift_distance_west = get_west_shift_distance(color);
  auto const west_attacks = (pawns.*bitshift_fn)(shift_distance_west) & ~Bitboard_constants::h_file;

  auto const east_attacks = (pawns.*bitshift_fn)(shift_distance_east) & ~Bitboard_constants::a_file;
  return west_attacks | east_attacks;
}

template <Color color>
constexpr Bitboard pawn_east_attacks(Bitboard pawns, Bitboard enemies)
{
  constexpr auto bitshift_fn = get_pawn_shift_fn(color);
  constexpr auto shift_distance = get_east_shift_distance(color);
  auto const east_attacks = (pawns.*bitshift_fn)(shift_distance) & ~Bitboard_constants::a_file & enemies;
  return east_attacks;
}

template <Color color>
constexpr Bitboard pawn_west_attacks(Bitboard pawns, Bitboard enemies)
{
  constexpr auto bitshift_fn = get_pawn_shift_fn(color);
  constexpr auto shift_distance = get_west_shift_distance(color);
  auto const west_attacks = (pawns.*bitshift_fn)(shift_distance) & ~Bitboard_constants::h_file & enemies;
  return west_attacks;
}

template <Color color>
constexpr void generate_castling_moves(Board const& board, std::vector<Move>& moves)
{
  constexpr Move short_castle_white{{4, 0}, {6, 0}, Piece::king, Piece::empty};
  constexpr Move long_castle_white{{4, 0}, {2, 0}, Piece::king, Piece::empty};
  constexpr Move short_castle_black{{4, 7}, {6, 7}, Piece::king, Piece::empty};
  constexpr Move long_castle_black{{4, 7}, {2, 7}, Piece::king, Piece::empty};

  constexpr Coordinates white_king_start_location{4, 0};
  constexpr Coordinates black_king_start_location{4, 7};

  auto const castling_rights = board.get_castling_rights();
  auto const occupied = board.get_occupied_squares();

  if (color == Color::white)
  {
    if (white_can_short_castle(castling_rights) &&
        (occupied & Bitboard_constants::short_castling_empty_squares_white).is_empty())
    {
      auto const attacks = Move_generator::get_all_attacked_squares(board, Color::black);
      if ((attacks & Bitboard_constants::short_castling_empty_squares_white).is_empty() &&
          !attacks.is_set(white_king_start_location))
      {
        moves.emplace_back(short_castle_white);
      }
    }

    if (white_can_long_castle(castling_rights) &&
        (occupied & Bitboard_constants::long_castling_empty_squares_white).is_empty() &&
        !occupied.is_set(Coordinates{1, 0}))
    {
      auto const attacks = Move_generator::get_all_attacked_squares(board, Color::black);
      if ((attacks & Bitboard_constants::long_castling_empty_squares_white).is_empty() &&
          !attacks.is_set(white_king_start_location))
      {
        moves.emplace_back(long_castle_white);
      }
    }
  }
  else
  {
    if (black_can_short_castle(castling_rights) &&
        (occupied & Bitboard_constants::short_castling_empty_squares_black).is_empty())
    {
      auto const attacks = Move_generator::get_all_attacked_squares(board, Color::white);
      if ((attacks & Bitboard_constants::short_castling_empty_squares_black).is_empty() &&
          !attacks.is_set(black_king_start_location))
      {
        moves.emplace_back(short_castle_black);
      }
    }

    if (black_can_long_castle(castling_rights) &&
        (occupied & Bitboard_constants::long_castling_empty_squares_black).is_empty() &&
        !occupied.is_set(Coordinates{1, 7}))
    {
      auto const attacks = Move_generator::get_all_attacked_squares(board, Color::white);
      if ((attacks & Bitboard_constants::long_castling_empty_squares_black).is_empty() &&
          !attacks.is_set(black_king_start_location))
      {
        moves.emplace_back(long_castle_black);
      }
    }
  }
}

//TODO: Refactor this to be more like get_all_attacked_squares
template <Color color>
constexpr void generate_piece_moves(Board const& board, std::vector<Move>& moves)
{
  // Parallel arrays that can be iterated together to get the piece type and the function that matches it
  constexpr std::array piece_types{Piece::rook, Piece::knight, Piece::bishop, Piece::queen, Piece::king};
  constexpr std::array piece_move_functions{&rook_attacks, &knight_attacks, &bishop_attacks, &queen_attacks,
                                            &king_attacks};

  auto const friends = ~board.get_all(color);
  auto const enemies = board.get_all(opposite_color(color));
  auto const occupied = board.get_occupied_squares();

  for (size_t i{0}; i < piece_types.size(); ++i)
  {
    auto pieces = board.get_piece_set(color, piece_types[i]);
    while (!pieces.is_empty())
    {
      auto const piece_location = pieces.pop_first_bit();
      auto possible_moves = piece_move_functions[i](Coordinates{piece_location}, occupied);
      possible_moves &= friends; // Throw out any moves to a square that is already occupied by our color
      auto possible_attacks = possible_moves & enemies;
      possible_moves &= ~possible_attacks; // Handle attacks separately

      while (!possible_moves.is_empty())
      {
        auto const end_location = possible_moves.pop_first_bit();
        moves.emplace_back(Coordinates{piece_location}, Coordinates{end_location}, piece_types[i], Piece::empty);
      }

      while (!possible_attacks.is_empty())
      {
        auto const end_location = possible_attacks.pop_first_bit();
        moves.emplace_back(Coordinates{piece_location}, Coordinates{end_location}, piece_types[i],
                           board.get_piece(Coordinates{end_location}));
      }
    }
  }
}

template <Color color>
constexpr void generate_piece_attacks(Board const& board, std::vector<Move>& moves)
{
  // Parallel arrays that can be iterated together to get the piece type and the function that matches it
  constexpr std::array piece_types{Piece::rook, Piece::knight, Piece::bishop, Piece::queen, Piece::king};
  constexpr std::array piece_move_functions{&rook_attacks, &knight_attacks, &bishop_attacks, &queen_attacks,
                                            &king_attacks};

  auto const not_friends = ~board.get_all(color);
  auto const enemies = board.get_all(opposite_color(color));
  auto const occupied = board.get_occupied_squares();
  for (size_t i{0}; i < piece_types.size(); ++i)
  {
    auto pieces = board.get_piece_set(color, piece_types[i]);
    while (!pieces.is_empty())
    {
      auto const piece_location = pieces.pop_first_bit();
      auto attacks = piece_move_functions[i](Coordinates{piece_location}, occupied);
      attacks &= not_friends; // Throw out any moves to a square that is already occupied by our color
      attacks &= enemies; // Throw out any moves that are not captures
      while (!attacks.is_empty())
      {
        auto const end_location = attacks.pop_first_bit();
        moves.emplace_back(Coordinates{piece_location}, Coordinates{end_location}, piece_types[i],
                           board.get_piece(Coordinates{end_location}));
      }
    }
  }
}

template <Color color>
constexpr void generate_pawn_attacks(Board const& board, std::vector<Move>& moves)
{
  // Handle east captures
  auto const east_offset = get_east_capture_offset(color);
  auto east_attacks =
    pawn_east_attacks<color>(board.get_piece_set(color, Piece::pawn), board.get_all(opposite_color(color)));
  while (!east_attacks.is_empty())
  {
    auto const location = east_attacks.pop_first_bit();
    Coordinates const from{location + east_offset};
    Coordinates const to{location};
    auto const victim = board.get_piece(to);
    if (to.y() == 0 || to.y() == 7)
    {
      moves.emplace_back(from, to, Piece::pawn, victim, Piece::bishop);
      moves.emplace_back(from, to, Piece::pawn, victim, Piece::knight);
      moves.emplace_back(from, to, Piece::pawn, victim, Piece::rook);
      moves.emplace_back(from, to, Piece::pawn, victim, Piece::queen);
    }
    else
    {
      moves.emplace_back(from, to, Piece::pawn, victim);
    }
  }

  // Handle west captures
  auto const west_offset = get_west_capture_offset(color);
  auto west_attacks =
    pawn_west_attacks<color>(board.get_piece_set(color, Piece::pawn), board.get_all(opposite_color(color)));
  while (!west_attacks.is_empty())
  {
    auto const location = west_attacks.pop_first_bit();
    Coordinates const from{location + west_offset};
    Coordinates const to{location};
    auto const victim = board.get_piece(to);
    if (to.y() == 0 || to.y() == 7)
    {
      moves.emplace_back(from, to, Piece::pawn, victim, Piece::bishop);
      moves.emplace_back(from, to, Piece::pawn, victim, Piece::knight);
      moves.emplace_back(from, to, Piece::pawn, victim, Piece::rook);
      moves.emplace_back(from, to, Piece::pawn, victim, Piece::queen);
    }
    else
    {
      moves.emplace_back(from, to, Piece::pawn, victim);
    }
  }
}

template <Color color>
constexpr void generate_pawn_moves(Board const& board, std::vector<Move>& moves)
{
  // For a one square pawn push, the starting square will be either 8 squares higher or lower than the ending square
  auto const offset_from_end_square = get_start_square_offset(color);

  auto short_advances =
    pawn_short_advances<color>(board.get_piece_set(color, Piece::pawn), board.get_occupied_squares());
  while (!short_advances.is_empty())
  {
    auto const location = short_advances.pop_first_bit();
    Coordinates from{location + offset_from_end_square};
    Coordinates to{location};
    if (to.y() == 0 || to.y() == 7)
    {
      moves.emplace_back(from, to, Piece::pawn, Piece::empty, Piece::bishop);
      moves.emplace_back(from, to, Piece::pawn, Piece::empty, Piece::knight);
      moves.emplace_back(from, to, Piece::pawn, Piece::empty, Piece::rook);
      moves.emplace_back(from, to, Piece::pawn, Piece::empty, Piece::queen);
    }
    else
    {
      moves.emplace_back(from, to, Piece::pawn, Piece::empty);
    }
  }

  auto long_advances = pawn_long_advances<color>(board.get_piece_set(color, Piece::pawn), board.get_occupied_squares());
  while (!long_advances.is_empty())
  {
    auto const location = long_advances.pop_first_bit();
    moves.emplace_back(Coordinates{location + 2 * offset_from_end_square}, Coordinates{location}, Piece::pawn,
                       Piece::empty);
  }

  // Handle en passant
  auto const east_offset = get_east_capture_offset(color);
  auto east_captures = pawn_east_attacks<color>(board.get_piece_set(color, Piece::pawn), board.get_en_passant_square());
  while (!east_captures.is_empty())
  {
    auto const location = east_captures.pop_first_bit();
    moves.emplace_back(Coordinates{location + east_offset}, Coordinates{location}, Piece::pawn, Piece::pawn,
                       Piece::empty, Move_type::en_passant);
  }

  auto const west_offset = get_west_capture_offset(color);
  auto west_captures = pawn_west_attacks<color>(board.get_piece_set(color, Piece::pawn), board.get_en_passant_square());
  while (!west_captures.is_empty())
  {
    auto const location = west_captures.pop_first_bit();
    moves.emplace_back(Coordinates{location + west_offset}, Coordinates{location}, Piece::pawn, Piece::pawn,
                       Piece::empty, Move_type::en_passant);
  }

  generate_pawn_attacks<color>(board, moves);
}

std::vector<Move> Move_generator::generate_pseudo_legal_moves(Board const& board)
{
  auto const color = board.get_active_color();
  std::vector<Move> pseudo_legal_moves;
  pseudo_legal_moves.reserve(218); // Max possible number of chess moves in a position

  if (color == Color::black)
  {
    generate_pawn_moves<Color::black>(board, pseudo_legal_moves);
    generate_castling_moves<Color::black>(board, pseudo_legal_moves);
    generate_piece_moves<Color::black>(board, pseudo_legal_moves);
  }
  else
  {
    generate_pawn_moves<Color::white>(board, pseudo_legal_moves);
    generate_castling_moves<Color::white>(board, pseudo_legal_moves);
    generate_piece_moves<Color::white>(board, pseudo_legal_moves);
  }

  return pseudo_legal_moves;
}

std::vector<Move> Move_generator::generate_legal_moves(Board const& board)
{
  auto const color = board.get_active_color();
  std::vector<Move> pseudo_legal_moves;
  pseudo_legal_moves.reserve(218); // Max possible number of chess moves in a position

  if (color == Color::black)
  {
    generate_pawn_moves<Color::black>(board, pseudo_legal_moves);
    generate_castling_moves<Color::black>(board, pseudo_legal_moves);
    generate_piece_moves<Color::black>(board, pseudo_legal_moves);
  }
  else
  {
    generate_pawn_moves<Color::white>(board, pseudo_legal_moves);
    generate_castling_moves<Color::white>(board, pseudo_legal_moves);
    generate_piece_moves<Color::white>(board, pseudo_legal_moves);
  }

  std::vector<Move> legal_moves;
  legal_moves.reserve(pseudo_legal_moves.size());
  Board tmp_board(board);

  //TODO: Erase/remove to skip second call to new?
  std::copy_if(pseudo_legal_moves.cbegin(), pseudo_legal_moves.cend(), std::back_inserter(legal_moves),
               [&](auto m)
               {
                 tmp_board = board;
                 return !tmp_board.move_results_in_check_destructive(m);
               });

  return legal_moves;
}

std::vector<Move> Move_generator::generate_pseudo_legal_attack_moves(Board const& board)
{
  auto const color = board.get_active_color();
  std::vector<Move> pseudo_legal_attacks;
  pseudo_legal_attacks.reserve(64);

  if (color == Color::black)
  {
    generate_piece_attacks<Color::black>(board, pseudo_legal_attacks);
    generate_pawn_attacks<Color::black>(board, pseudo_legal_attacks);
  }
  else
  {
    generate_piece_attacks<Color::white>(board, pseudo_legal_attacks);
    generate_pawn_attacks<Color::white>(board, pseudo_legal_attacks);
  }

  return pseudo_legal_attacks;
}

std::vector<Move> Move_generator::generate_legal_attack_moves(Board const& board)
{
  auto pseudo_legal_attacks = generate_pseudo_legal_attack_moves(board);

  std::vector<Move> legal_attacks;
  legal_attacks.reserve(64);
  Board tmp_board(board);
  std::copy_if(pseudo_legal_attacks.cbegin(), pseudo_legal_attacks.cend(), std::back_inserter(legal_attacks),
               [&](auto m)
               {
                 tmp_board = board;
                 return !tmp_board.move_results_in_check_destructive(m);
               });

  return legal_attacks;
}

Bitboard Move_generator::get_all_attacked_squares(Board const& board, Color attacking_color)
{
  auto const occupied = board.get_occupied_squares();
  Bitboard attacked_squares;

  auto pieces = board.get_piece_set(attacking_color, Piece::rook);
  while (!pieces.is_empty())
  {
    auto const piece_location = pieces.pop_first_bit();
    attacked_squares |= rook_attacks(Coordinates{piece_location}, occupied);
  }

  pieces = board.get_piece_set(attacking_color, Piece::knight);
  while (!pieces.is_empty())
  {
    auto const piece_location = pieces.pop_first_bit();
    attacked_squares |= knight_attacks(Coordinates{piece_location}, occupied);
  }

  pieces = board.get_piece_set(attacking_color, Piece::bishop);
  while (!pieces.is_empty())
  {
    auto const piece_location = pieces.pop_first_bit();
    attacked_squares |= bishop_attacks(Coordinates{piece_location}, occupied);
  }

  pieces = board.get_piece_set(attacking_color, Piece::queen);
  while (!pieces.is_empty())
  {
    auto const piece_location = pieces.pop_first_bit();
    attacked_squares |= queen_attacks(Coordinates{piece_location}, occupied);
  }

  MY_ASSERT(board.get_piece_set(attacking_color, Piece::king).occupancy() == 1,
            "Board should never have two kings of the same color");
  attacked_squares |=
    king_attacks(Coordinates{board.get_piece_set(attacking_color, Piece::king).bitscan_forward()}, occupied);

  auto const pawn_attacks = (attacking_color == Color::black) ?
                              pawn_potential_attacks<Color::black>(board.get_piece_set(attacking_color, Piece::pawn)) :
                              pawn_potential_attacks<Color::white>(board.get_piece_set(attacking_color, Piece::pawn));

  return attacked_squares | pawn_attacks;
}

bool Move_generator::is_square_attacked(Board const& board, Color attacking_color, Bitboard attacked_square)
{
  MY_ASSERT(attacked_square.occupancy() == 1, "Attacked square can only have one set square");

  auto const occupied = board.get_occupied_squares();
  Coordinates attacked_location(attacked_square.bitscan_forward());

  if (auto attacks = rook_attacks(attacked_location, occupied);
      !(attacks &
        (board.get_piece_set(attacking_color, Piece::rook) | board.get_piece_set(attacking_color, Piece::queen)))
         .is_empty())
  {
    return true;
  }

  if (auto attacks = bishop_attacks(attacked_location, occupied);
      !(attacks &
        (board.get_piece_set(attacking_color, Piece::bishop) | board.get_piece_set(attacking_color, Piece::queen)))
         .is_empty())
  {
    return true;
  }

  if (auto attacks = knight_attacks(attacked_location, occupied);
      !(attacks & board.get_piece_set(attacking_color, Piece::knight)).is_empty())
  {
    return true;
  }

  if (auto attacks = king_attacks(attacked_location, occupied);
      !(attacks & board.get_piece_set(attacking_color, Piece::king)).is_empty())
  {
    return true;
  }

  if (attacking_color == Color::black)
  {
    return !(pawn_potential_attacks<Color::black>(board.get_piece_set(attacking_color, Piece::pawn)) & attacked_square)
              .is_empty();
  }
  return !(pawn_potential_attacks<Color::white>(board.get_piece_set(attacking_color, Piece::pawn)) & attacked_square)
            .is_empty();
}

// Faster than generating all moves and checking if the list is empty
bool Move_generator::has_any_legal_moves(Board const& board)
{
  // Parallel arrays that can be iterated together to get the piece type and the function that matches it
  constexpr static std::array piece_types{Piece::king, Piece::queen, Piece::knight, Piece::bishop, Piece::rook};
  constexpr static std::array piece_move_functions{&king_attacks, &queen_attacks, &knight_attacks, &bishop_attacks,
                                                   &rook_attacks};

  Board tmp_board{board};
  auto const color = board.get_active_color();
  for (size_t i{0}; i < piece_types.size(); ++i)
  {
    auto pieces = board.get_piece_set(color, piece_types[i]);
    while (!pieces.is_empty())
    {
      auto const piece_location = pieces.pop_first_bit();
      auto possible_moves = piece_move_functions[i](Coordinates{piece_location}, board.get_occupied_squares());
      possible_moves &= ~board.get_all(color); // Throw out any moves to a square that is already occupied by our color

      while (!possible_moves.is_empty())
      {
        auto const end_location = possible_moves.pop_first_bit();
        tmp_board = board;
        if (!tmp_board.move_results_in_check_destructive({Coordinates{piece_location}, Coordinates{end_location},
                                                          piece_types[i], board.get_piece(Coordinates{end_location})}))
        {
          return true;
        }
      }
    }
  }

  std::vector<Move> pseudo_legal_moves;
  pseudo_legal_moves.reserve(32);

  if (color == Color::black)
  {
    generate_pawn_moves<Color::black>(board, pseudo_legal_moves);
  }
  else
  {
    generate_pawn_moves<Color::white>(board, pseudo_legal_moves);
  }
  return std::any_of(pseudo_legal_moves.cbegin(), pseudo_legal_moves.cend(),
                     [&](auto m)
                     {
                       tmp_board = board;
                       return !tmp_board.move_results_in_check_destructive(m);
                     });
}

uint64_t Move_generator::perft(int depth, Board& board, std::atomic_flag& is_cancelled)
{
  uint64_t nodes{0};

  if (depth == 0)
  {
    return uint64_t{1};
  }

  auto color = board.get_active_color();
  auto moves = Move_generator::generate_pseudo_legal_moves(board);
  for (auto m : moves)
  {
    auto tmp_board = Board{board};
    [[maybe_unused]] auto succeeded = tmp_board.move_no_verify(m);
    MY_ASSERT(succeeded, "Invalid move");

    if (!tmp_board.is_in_check(color))
    {
      nodes += perft(depth - 1, tmp_board, is_cancelled);
    }
    if (is_cancelled.test())
    {
      return nodes;
    }
  }

  return nodes;
}
