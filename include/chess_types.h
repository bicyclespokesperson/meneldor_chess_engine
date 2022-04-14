#ifndef CHESS_TYPES_H
#define CHESS_TYPES_H

constexpr int positive_inf = 100'000;
constexpr int negative_inf = -positive_inf;
constexpr int c_max_supported_depth{1000};
constexpr int c_max_non_mate_score{positive_inf - c_max_supported_depth};
constexpr int c_min_non_mate_score{negative_inf + c_max_supported_depth};

static_assert(positive_inf == -negative_inf, "Values should be inverses of each other");

static constexpr int32_t c_board_dimension{8};
static constexpr int32_t c_board_dimension_squared{c_board_dimension * c_board_dimension};
static const std::string c_start_position_fen{"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"};

enum class Color : uint8_t
{
  black = 0,
  white,
  _count,
};

enum class Piece : uint8_t
{
  black = static_cast<uint8_t>(Color::black), // Any black piece
  white = static_cast<uint8_t>(Color::white), // Any white piece
  pawn,
  knight,
  bishop,
  rook,
  queen,
  king,
  empty,
  _count,
};

enum class Move_type : uint8_t
{
  null = 0,
  normal,
  en_passant,
  _count,
};

enum class Game_state
{
  in_progress = 0,
  draw,
  white_victory,
  black_victory,
};

class Threefold_repetition_detector
{
public:
  Threefold_repetition_detector() = default;

  // Call this after every move, passing it a fen str of the current game
  // returns true if the game is a draw due to threefold repetition
  bool add_fen(std::string_view fen);

  bool is_drawn() const;

private:
  std::unordered_map<std::string, uint8_t> m_previous_positions;
  bool m_is_drawn{false};
};

constexpr Piece to_piece_enum(Color c)
{
  return static_cast<Piece>(c);
}

constexpr Color opposite_color(Color color)
{
  return static_cast<Color>(1 - static_cast<uint8_t>(color));
}

Piece from_char(char c);

std::ostream& operator<<(std::ostream& os, Piece const& self);

std::ostream& operator<<(std::ostream& os, Color const& self);

#endif // CHESS_TYPES_H
