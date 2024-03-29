#include "board.h"
#include "move_generator.h"
#include "my_assert.h"
#include "utils.h"

namespace rs = std::ranges;
namespace Meneldor
{

bool piece_can_move(Coordinates from, Coordinates to, Board const& board);

bool bishop_can_move(Coordinates from, Coordinates to, Board const& board)
{
  if (!(board.is_clear_diagonal(from, to)))
  {
    return false;
  }

  return true;
}

bool knight_can_move(Coordinates from, Coordinates to, Board const& /* board */)
{
  // Make sure the move is either two vertical and one horizontal
  if (std::abs(from.y() - to.y()) == 2 && std::abs(from.x() - to.x()) == 1)
  {
    return true;
  }

  // Or two horizontal and one vertical
  if (std::abs(from.x() - to.x()) == 2 && std::abs(from.y() - to.y()) == 1)
  {
    return true;
  }

  return false;
}

bool rook_can_move(Coordinates from, Coordinates to, Board const& board)
{
  if (!(board.is_clear_horizontal(from, to) || board.is_clear_vertical(from, to)))
  {
    return false;
  }

  return true;
}

bool queen_can_move(Coordinates from, Coordinates to, Board const& board)
{
  return rook_can_move(from, to, board) || bishop_can_move(from, to, board);
}

bool king_can_move(Coordinates from, Coordinates to, Board const& board)
{
  if (board.distance_between(from, to) == 1)
  {
    return true;
  }

  // Check if castling is allowed
  if (from.y() == to.y() && std::abs(from.x() - to.x()) == 2)
  {
    if (!board.can_castle_to(to))
    {
      return false; // One of the pieces necessary for castling has already
        // moved
    }

    Coordinates transit_square{static_cast<int32_t>((from.x() + to.x()) / 2), from.y()};

    if (board.is_occupied(transit_square))
    {
      return false;
    }

    // Make sure the knight is out of the way when long castling
    if (c1 == to && board.is_occupied(b1))
    {
      return false;
    }

    if (c8 == to && board.is_occupied(b8))
    {
      return false;
    }

    auto const color = board.get_piece_color(from);
    auto attacked_squares = Move_generator::get_all_attacked_squares(board, opposite_color(color));
    if (attacked_squares.is_set(from) || attacked_squares.is_set(to) || attacked_squares.is_set(transit_square))
    {
      return false; // Illegal to castle out of, through, or into check
    }

    return true;
  }

  return false;
}

bool pawn_can_move(Coordinates from, Coordinates to, Board const& board)
{
  auto const color = board.get_piece_color(from);
  bool const is_white = color == Color::white;

  // A pawn can move two spaces on its first move
  int const max_distance = [&]
  {
    if ((is_white && from.y() == 1) || (!is_white && from.y() == 6))
    {
      return 2;
    }
    return 1;
  }();

  // Make sure the distance of the move is not greater than 1, or 2 if the
  // piece has not yet moved.
  int const move_distance = board.distance_between(from, to);
  if (move_distance > max_distance)
  {
    return false;
  }

  // Make sure the pawn is moving forward
  if ((is_white && to.y() > from.y()) || (!is_white && to.y() < from.y()))
  {
    // Ensure the space ahead of the pawn is clear and vertical
    // This also prevents moves of zero spaces, since it cannot move to
    // a space occupied by itself.
    if (board.is_clear_vertical(from, to) && !(board.is_occupied(to)))
    {
      return true;
    }

    // If the square is diagonally forward and occupied by an opponent
    if (move_distance == 1 && board.is_clear_diagonal(from, to) &&
        (board.is_occupied(to) || board.get_en_passant_square().is_set(to)))
    {
      return true;
    }
  }

  return false;
}

bool piece_can_move(Coordinates from, Coordinates to, Board const& board)
{
  if (!board.is_occupied(from))
  {
    return false;
  }

  auto const color = board.get_piece_color(from);

  if (board.get_all(color).is_set(to))
  {
    return false;
  }

  switch (board.get_piece(from))
  {
    case Piece::pawn:
      return pawn_can_move(from, to, board);
    case Piece::knight:
      return knight_can_move(from, to, board);
    case Piece::bishop:
      return bishop_can_move(from, to, board);
    case Piece::rook:
      return rook_can_move(from, to, board);
    case Piece::queen:
      return queen_can_move(from, to, board);
    case Piece::king:
      return king_can_move(from, to, board);
    default:
      return false;
  }
}

// This array can be indexed by color
static constexpr std::array en_passant_y_offsets{1, -1};
constexpr Coordinates en_passant_capture_location(Color capturing_color, Coordinates end_square)
{
  return Coordinates{end_square.x(), end_square.y() + en_passant_y_offsets[static_cast<uint8_t>(capturing_color)]};
}

bool is_en_passant(Piece piece, Coordinates from, Coordinates to, Board const& board)
{
  if (piece == Piece::pawn && board.is_clear_diagonal(from, to) && !board.is_occupied(to))
  {
    return true;
  }
  return false;
}

Board::Board()
{
  reset();
}

Board::Board(int) : m_rights{c_castling_rights_none}
{
}

Bitboard Board::get_piece_set(Color color, Piece piece) const
{
  return m_bitboards[static_cast<uint8_t>(color)] & m_bitboards[static_cast<uint8_t>(piece)];
}

Bitboard Board::get_all(Piece piece) const
{
  return m_bitboards[static_cast<uint8_t>(piece)];
}

Bitboard Board::get_all(Color color) const
{
  return m_bitboards[static_cast<uint8_t>(color)];
}

Bitboard Board::get_en_passant_square() const
{
  return m_en_passant_square;
}

Bitboard Board::get_occupied_squares() const
{
  return m_bitboards[static_cast<uint8_t>(Color::white)] | m_bitboards[static_cast<uint8_t>(Color::black)];
}

bool Board::is_occupied(Coordinates square) const
{
  return get_occupied_squares().is_set(square);
}

Color Board::get_piece_color(Coordinates square) const
{
  MY_ASSERT(is_occupied(square), "This function can only be called for occupied squares");

  return get_all(Color::white).is_set(square) ? Color::white : Color::black;
}

Piece Board::get_piece(Coordinates square) const
{
  if (get_all(Piece::pawn).is_set(square))
  {
    return Piece::pawn;
  }

  if (get_all(Piece::bishop).is_set(square))
  {
    return Piece::bishop;
  }

  if (get_all(Piece::knight).is_set(square))
  {
    return Piece::knight;
  }

  if (get_all(Piece::rook).is_set(square))
  {
    return Piece::rook;
  }

  if (get_all(Piece::queen).is_set(square))
  {
    return Piece::queen;
  }

  if (get_all(Piece::king).is_set(square))
  {
    return Piece::king;
  }

  return Piece::empty;
}

Castling_rights Board::get_castling_rights() const
{
  return m_rights;
}

void Board::perform_move_(Move m, Coordinates capture_location)
{
  MY_ASSERT(get_piece(capture_location) == m.victim(), "Move is in an invalid state");

  auto const color = get_active_color();

  if (is_occupied(m.to()) || capture_location != m.to())
  {
    remove_piece_(opposite_color(color), m.victim(), capture_location);
  }

  add_piece_(color, m.piece(), m.to());
  remove_piece_(color, m.piece(), m.from());
}

void Board::unperform_move_(Color color, Move m)
{
  MY_ASSERT(is_occupied(m.to()) && !is_occupied(m.from()),
            "This function can only be called after a move has been performed");
  MY_ASSERT(m.type() != Move_type::en_passant || !get_en_passant_square().is_empty(),
            "Must have an en passant square for en passant move");

  remove_piece_(color, m.piece(), m.to());
  add_piece_(color, m.piece(), m.from());

  if (m.victim() != Piece::empty)
  {
    auto const capture_location{
      (m.type() == Move_type::en_passant) ?
        en_passant_capture_location(color, Coordinates{get_en_passant_square().bitscan_forward()}) :
        m.to()};
    add_piece_(opposite_color(color), m.victim(), capture_location);
  }
}

bool Board::undo_move(Move m, Bitboard en_passant_square, Castling_rights rights, uint8_t halfmove_clock)
{
  MY_ASSERT(get_piece(m.to()) == m.piece(), "Move has incorrect piece");
  MY_ASSERT(m.piece() != Piece::empty, "Cannot undo move without a piece on target square");

  auto const color = opposite_color(get_active_color());
  MY_ASSERT(color == get_piece_color(m.to()), "Cannot undo move for current player's turn");

  MY_ASSERT((m.promotion() == Piece::empty) || (m.to().y() == 0 || m.to().y() == 7),
            "Promotion move must end on back rank");

  m_zhash.update_en_passant_square(m_en_passant_square);
  m_en_passant_square = en_passant_square;
  m_zhash.update_en_passant_square(m_en_passant_square);

  unperform_move_(color, m);

  if (m.piece() == Piece::king && distance_between(m.to(), m.from()) == 2)
  {
    auto const rook_move = find_castling_rook_move_(m.to());
    unperform_move_(color, rook_move);
  }

  if (m.promotion() != Piece::empty)
  {
    remove_piece_(color, m.promotion(), m.from());
    add_piece_(color, Piece::pawn, m.from());
  }

  m_zhash.update_castling_rights(m_rights);
  m_rights = rights;
  m_zhash.update_castling_rights(m_rights);

  if (color == Color::black)
  {
    --m_fullmove_count;
  }
  if (m.piece() == Piece::pawn || m.victim() != Piece::empty || m.promotion() != Piece::empty)
  {
    m_halfmove_clock = halfmove_clock;
  }
  else
  {
    MY_ASSERT(m_halfmove_clock > 0, "Halfmove clock should never go negative");
    --m_halfmove_clock;
  }

  m_zhash.update_player_to_move();
  m_active_color = opposite_color(m_active_color);
  MY_ASSERT(validate_(), "Board is in an incorrect state after move");

  return false;
}

bool Board::move_results_in_check_destructive(Move m)
{
  auto const color = get_active_color();

  auto capture_location = m.to();
  if (m.type() == Move_type::en_passant)
  {
    capture_location = en_passant_capture_location(color, m.to());
  }

  perform_move_(m, capture_location);
  return is_in_check(color);
}

tl::expected<void, std::string> Board::try_move(Move m)
{
  MY_ASSERT(m.piece() == get_piece(m.from()), "Move has incorrect moving piece");

  if (m.piece() == Piece::empty)
  {
    return tl::unexpected(std::string{"Empty start square"});
  }

  auto const color = get_active_color();

  if (get_piece_color(m.from()) != color)
  {
    return tl::unexpected(std::string{"Incorrect moving color"});
  }

  if (!piece_can_move(m.from(), m.to(), *this))
  {
    return tl::unexpected(std::string{"Piece doesn't move like that"});
  }

  // Ensure that a promotion square is present iff we have a pawn move to the
  // back rank
  if ((m.piece() == Piece::pawn && (m.to().y() == 0 || m.to().y() == 7)) != (m.promotion() != Piece::empty))
  {
    return tl::unexpected(std::string{"Invalid promotion"});
  }

  if (m.promotion() == Piece::king || m.promotion() == Piece::pawn)
  {
    return tl::unexpected(std::string{"Cannot promote to that piece"});
  }

  constexpr static bool skip_check_detection{false};
  return move_no_verify(m, skip_check_detection) ? tl::expected<void, std::string>{} :
                                                   tl::unexpected(std::string{"Move leaves king in check"});
}

bool Board::move_no_verify(Move m, bool skip_check_detection)
{
  auto const color = get_active_color();

  if (m.type() != Move_type::null)
  {
    auto capture_location = m.to();
    if (m.type() == Move_type::en_passant)
    {
      capture_location = en_passant_capture_location(color, m.to());
    }

    perform_move_(m, capture_location);
    if (!skip_check_detection && is_in_check(color))
    {
      unperform_move_(color, m);
      return false;
    }

    m_zhash.update_castling_rights(m_rights);
    update_castling_rights_(color, m.piece(), m);
    m_zhash.update_castling_rights(m_rights);

    // Move rook if the move was a castle
    if (m.piece() == Piece::king && distance_between(m.from(), m.to()) == 2)
    {
      auto const rook_move = find_castling_rook_move_(m.to());
      perform_move_(rook_move, rook_move.to());
    }

    // Promote pawn if it reached the last rank
    if (m.promotion() != Piece::empty)
    {
      remove_piece_(color, Piece::pawn, m.to());
      add_piece_(color, m.promotion(), m.to());
    }

    if (m.piece() == Piece::pawn || m.victim() != Piece::empty)
    {
      m_halfmove_clock = 0;
    }
    else
    {
      ++m_halfmove_clock;
    }
  }

  // Set en passant square if applicable
  m_zhash.update_en_passant_square(m_en_passant_square);
  m_en_passant_square.unset_all();
  if (m.piece() == Piece::pawn && distance_between(m.from(), m.to()) == 2)
  {
    int const offset = (color == Color::white) ? -1 : 1;
    m_en_passant_square.set_square(Coordinates{m.to().x(), m.to().y() + offset});
  }
  m_zhash.update_en_passant_square(m_en_passant_square);

  if (color == Color::black)
  {
    ++m_fullmove_count;
  }

  m_active_color = opposite_color(color);
  m_zhash.update_player_to_move();

  MY_ASSERT(validate_(), "Board is in an incorrect state after move");
  return true;
}

tl::expected<void, std::string> Board::try_move_algebraic(std::string_view move_str)
{
  return move_from_algebraic(move_str, get_active_color())
    .and_then(
      [&](auto const m)
      {
        return try_move(m);
      });
}

tl::expected<void, std::string> Board::try_move_uci(std::string_view move_str)
{
  return move_from_uci(std::string{move_str})
    .and_then(
      [&](auto const m)
      {
        return try_move(m);
      });
}

Color Board::get_active_color() const
{
  return m_active_color;
}

uint8_t Board::get_halfmove_clock() const
{
  return m_halfmove_clock;
}

uint8_t Board::get_move_count() const
{
  return m_fullmove_count;
}

bool Board::has_sufficient_material(Color color) const
{
  if (!get_piece_set(color, Piece::pawn).is_empty() || !get_piece_set(color, Piece::rook).is_empty() ||
      !get_piece_set(color, Piece::queen).is_empty())
  {
    return true;
  }

  if ((get_piece_set(color, Piece::bishop).occupancy() + get_piece_set(color, Piece::knight).occupancy()) >= 2)
  {
    return true;
  }

  return false;
}

Zobrist_hash Board::get_hash_key() const
{
  return m_zhash;
}

Move Board::find_castling_rook_move_(Coordinates king_destination) const
{
  if (king_destination == c1)
  {
    return {a1, d1, Piece::rook, Piece::empty};
  }
  if (king_destination == g1)
  {
    return {h1, f1, Piece::rook, Piece::empty};
  }
  if (king_destination == c8)
  {
    return {a8, d8, Piece::rook, Piece::empty};
  }
  if (king_destination == g8)
  {
    return {h8, f8, Piece::rook, Piece::empty};
  }

  MY_ASSERT(false, "Invalid castling move");
  return {{0, 0}, {0, 0}, Piece::empty, Piece::empty};
}

bool Board::can_castle_to(Coordinates dest) const
{
  if (dest == c1)
  {
    return white_can_long_castle(m_rights);
  }

  if (dest == g1)
  {
    return white_can_short_castle(m_rights);
  }

  if (dest == c8)
  {
    return black_can_long_castle(m_rights);
  }

  if (dest == g8)
  {
    return black_can_short_castle(m_rights);
  }

  return false;
}

void Board::update_castling_rights_(Color color, Piece piece, Move m)
{
  if (piece == Piece::king)
  {
    if (color == Color::black)
    {
      set_black_short_castle_false(m_rights);
      set_black_long_castle_false(m_rights);
    }
    else
    {
      set_white_short_castle_false(m_rights);
      set_white_long_castle_false(m_rights);
    }
  }
  else
  {
    // TODO: Could this be a lookup table, maybe with some sort of perfect hashing scheme?
    auto const to = m.to();
    auto const from = m.from();
    if (to == a1 || from == a1)
    {
      set_white_long_castle_false(m_rights);
    }
    if (to == h1 || from == h1)
    {
      set_white_short_castle_false(m_rights);
    }
    if (to == a8 || from == a8)
    {
      set_black_long_castle_false(m_rights);
    }
    if (to == h8 || from == h8)
    {
      set_black_short_castle_false(m_rights);
    }
  }
}

/**
 * Calculates the distance between two squares on the board
 * @param from The first square
 * @param to The second square
 * @return The resulting distance, or -1 if there is no line between the two
 * squares
 */
int Board::distance_between(Coordinates from, Coordinates to) const
{
  // If the squares are on the same vertical, return the difference
  // in their horizontals
  if (from.x() == to.x())
  {
    return std::abs(from.y() - to.y());
  }

  // If the squares are on the same horizontal, return the difference
  // in their verticals
  if (from.y() == to.y())
  {
    return std::abs(from.x() - to.x());
  }

  // If the squares are on the same diagonal, their x and y differences
  // will be equal, so return their X difference
  if (std::abs(from.x() - to.x()) == std::abs(from.y() - to.y()))
  {
    return std::abs(from.x() - to.x());
  }

  return -1;
}

void Board::reset()
{
  *this = *from_fen(c_start_position_fen);
  MY_ASSERT(validate_(), "Board is in an incorrect state");
}

bool Board::is_clear_vertical(Coordinates from, Coordinates to) const
{
  // Set up the counter for the while loop.
  // Assume we are going to start from one space ahead of the current square
  // and walk straight until we hit the destination square

  if (from.x() != to.x())
  {
    return false;
  }

  auto top = to;
  auto bottom = from;
  // Set up the squares so we always walk up.
  // So if "to" is above "from", swap them.
  if (to.y() < from.y())
  {
    std::swap(bottom, top);
  }

  // Walk along the board and if we find an occupied space, exit the loop
  // and return false.
  for (int i = bottom.y() + 1; i < top.y(); i++)
  {
    if (is_occupied({from.x(), i}))
    {
      return false;
    }
  }

  return true;
}

bool Board::is_clear_horizontal(Coordinates from, Coordinates to) const
{
  // Set up the counter for the while loop.
  // Assume we are going to start from one space ahead of the current square
  // and walk straight until we hit the destination square

  if (from.y() != to.y())
  {
    return false;
  }

  // Set up the squares so we always walk up.
  // So if to is above from, swap them.
  auto right = to;
  auto left = from;
  if (to.x() < from.x())
  {
    std::swap(left, right);
  }

  // Walk along the board and if we find an occupied space, exit the loop
  // and return false.
  for (int i = left.x() + 1; i < right.x(); i++)
  {
    if (is_occupied({i, from.y()}))
    {
      return false;
    }
  }
  return true;
}

bool Board::is_clear_diagonal(Coordinates from, Coordinates to) const
{
  if (std::abs(from.x() - to.x()) != std::abs(from.y() - to.y()))
  {
    return false;
  }

  // Ensure we are walking right
  auto left = from;
  auto right = to;
  if (from.x() > to.x())
  {
    std::swap(left, right);
  }

  // Assume that we are walking up
  int direction = 1;
  if (left.y() > right.y())
  {
    direction = -1;
  }

  // Walk from "left" to "right"
  for (int i = 1; i < right.x() - left.x(); i++)
  {
    // Check to see if square is occupied
    if (is_occupied({left.x() + i, left.y() + direction * i}))
    {
      return false;
    }
  }
  return true;
}

bool Board::is_in_check(Color color) const
{
  Bitboard king_location = get_piece_set(color, Piece::king);
  return Move_generator::is_square_attacked(*this, opposite_color(color), king_location);
}

Game_state Board::calc_game_state() const
{
  // Doesn't check repetition, 50 move rule, or insufficient material for performance reasons.
  // These can be handled more efficienly by the engine.

  auto const color = get_active_color();

  if (Move_generator::has_any_legal_moves(*this))
  {
    return Game_state::in_progress;
  }

  if (is_in_check(color))
  {
    return (color == Color::black) ? Game_state::white_victory : Game_state::black_victory;
  }

  return Game_state::draw;
}

void Board::add_piece_(Color color, Piece piece, Coordinates to_add)
{
  MY_ASSERT(!m_bitboards[static_cast<uint8_t>(color)].is_set(to_add), "Cannot add piece that is already present");
  MY_ASSERT(!m_bitboards[static_cast<uint8_t>(piece)].is_set(to_add), "Cannot add piece that is already present");

  m_bitboards[static_cast<uint8_t>(color)].set_square(to_add);
  m_bitboards[static_cast<uint8_t>(piece)].set_square(to_add);
  m_zhash.update_piece_location(color, piece, to_add);
}

void Board::remove_piece_(Color color, Piece piece, Coordinates to_remove)
{
  MY_ASSERT(m_bitboards[static_cast<uint8_t>(piece)].is_set(to_remove), "Cannot remove piece that is not present");

  m_bitboards[static_cast<uint8_t>(color)].unset_square(to_remove);
  m_bitboards[static_cast<uint8_t>(piece)].unset_square(to_remove);
  m_zhash.update_piece_location(color, piece, to_remove);
}

bool Board::validate_() const
{
  if ((get_piece_set(Color::black, Piece::pawn) | get_piece_set(Color::black, Piece::bishop) |
       get_piece_set(Color::black, Piece::knight) | get_piece_set(Color::black, Piece::rook) |
       get_piece_set(Color::black, Piece::queen) | get_piece_set(Color::black, Piece::king)) != get_all(Color::black))
  {
    return false;
  }

  if ((get_piece_set(Color::white, Piece::pawn) | get_piece_set(Color::white, Piece::bishop) |
       get_piece_set(Color::white, Piece::knight) | get_piece_set(Color::white, Piece::rook) |
       get_piece_set(Color::white, Piece::queen) | get_piece_set(Color::white, Piece::king)) != get_all(Color::white))
  {
    return false;
  }

  // Ensure that the same bit is not set for multiple different piece types
  constexpr static std::array piece_types{Piece::pawn, Piece::knight, Piece::bishop,
                                          Piece::rook, Piece::queen,  Piece::king};
  for (auto piece1 : piece_types)
  {
    for (auto piece2 : piece_types)
    {
      if (piece1 != piece2)
      {
        if (!(get_all(piece1) & get_all(piece2)).is_empty())
        {
          return false;
        }
      }
    }
  }

  Zobrist_hash new_hash{*this};
  if (new_hash != m_zhash)
  {
    return false;
  }

  return true;
}

std::vector<Coordinates> Board::find_pieces_that_can_move_to(Piece piece, Color color, Coordinates target_square) const
{
  std::vector<Coordinates> candidates;
  auto locations = get_piece_set(color, piece);

  for (auto index : locations)
  {
    Coordinates location{index};
    if (piece_can_move(location, target_square, *this))
    {
      candidates.push_back(location);
    }
  }

  return candidates;
}

tl::expected<Move, std::string> Board::move_from_uci(std::string move_str) const
{
  move_str.erase(rs::remove_if(move_str, isspace).begin(), move_str.end());
  rs::transform(move_str, move_str.begin(),
                [](char c)
                {
                  return std::toupper(c, std::locale());
                });

  Piece promotion_result{Piece::empty};
  if (move_str.size() == 5 && !isdigit(move_str.back()))
  {
    promotion_result = from_char(std::toupper(move_str.back(), std::locale()));
    move_str.resize(move_str.size() - 1);
  }

  if (move_str.size() < 4)
  {
    return tl::unexpected(std::string{"Incorrect move string length"});
  }

  auto from = Coordinates::from_str(move_str);
  auto to = Coordinates::from_str({move_str.c_str() + 2, 2});

  if (from && to)
  {
    auto const moving_piece = get_piece(*from);
    auto const is_ep = is_en_passant(moving_piece, *from, *to, *this);
    auto const victim_piece = is_ep ? Piece::pawn : get_piece(*to);
    auto const move_type = is_ep ? Move_type::en_passant : Move_type::normal;
    return Move{*from, *to, moving_piece, victim_piece, promotion_result, move_type};
  }

  return tl::unexpected(std::string{"Invalid move"});
}

tl::expected<Move, std::string> Board::move_from_algebraic(std::string_view move_param, Color color) const
{
  std::string move_str;
  rs::copy_if(move_param, std::back_inserter(move_str),
              [&](char c)
              {
                constexpr std::string_view chars = "x+#?!";
                return !(std::isspace(c) || chars.contains(c));
              });

  if (move_str.size() < 2)
  {
    return {};
  }

  Piece promotion_result{Piece::empty};
  if (auto index = move_str.find('='); index != std::string::npos)
  {
    if (index + 1 >= move_str.size())
    {
      return tl::unexpected(std::string{"Invalid promotion target"});
    }
    promotion_result = from_char(std::toupper(move_str[index + 1], std::locale()));
    move_str.resize(move_str.size() - 2);
  }

  if (move_str == "O-O" || move_str == "0-0")
  {
    if (color == Color::white)
    {
      return Move{e1, g1, Piece::king, Piece::empty};
    }
    else
    {
      return Move{e8, g8, Piece::king, Piece::empty};
    }
  }

  if (move_str == "O-O-O" || move_str == "0-0-0")
  {
    if (color == Color::white)
    {
      return Move{e1, c1, Piece::king, Piece::empty};
    }
    else
    {
      return Move{e8, c8, Piece::king, Piece::empty};
    }
  }

  // Read the square from the last two characters in the string
  auto target_square = Coordinates::from_str({move_str.c_str() + move_str.size() - 2, 2});
  move_str.resize(move_str.size() - 2); // Drop target square from string

  auto const piece = [&]
  {
    // Handle pawn move case (ex: e4, xe4, fxe4)
    if (move_str.empty() || islower(move_str[0]))
    {
      return Piece::pawn;
    }

    // Handle piece move case (ex: Bc4)
    auto piece = from_char(move_str[0]);
    move_str = move_str.substr(1);

    return piece;
  }();

  if (piece == Piece::empty)
  {
    return tl::unexpected(std::string{"Invalid piece for move"});
  }

  auto candidates = find_pieces_that_can_move_to(piece, color, *target_square);
  if (candidates.empty())
  {
    return tl::unexpected(std::string{"No piece can perform move"});
  }

  if (candidates.size() == 1)
  {
    // Exactly one piece can move to the target square
    auto const is_ep = is_en_passant(piece, candidates.front(), *target_square, *this);
    auto const victim = is_ep ? Piece::pawn : get_piece(*target_square);
    auto const move_type = is_ep ? Move_type::en_passant : Move_type::normal;
    return Move{candidates.front(), *target_square, piece, victim, promotion_result, move_type};
  }

  if (move_str.empty())
  {
    return tl::unexpected(std::string{"Too many pieces can perform move"});
  }

  if (isalpha(move_str[0]))
  {
    // Drop candidates that are not on the correct column
    auto start_column = static_cast<int32_t>(move_str[0] - 'a');
    candidates.erase(rs::remove_if(candidates,
                                   [start_column](Coordinates piece_loc)
                                   {
                                     return piece_loc.x() != start_column;
                                   })
                       .begin(),
                     candidates.end());

    move_str = move_str.substr(1);
    if (candidates.size() == 1)
    {
      // Exactly one piece can move to the target square

      auto const is_ep = is_en_passant(piece, candidates.front(), *target_square, *this);
      auto const victim = is_ep ? Piece::pawn : get_piece(*target_square);
      auto const move_type = is_ep ? Move_type::en_passant : Move_type::normal;
      return Move{candidates.front(), *target_square, piece, victim, promotion_result, move_type};
    }
  }

  if (move_str.empty())
  {
    return tl::unexpected(std::string{"Too many pieces can perform move"});
  }

  if (isdigit(move_str[0]))
  {
    // Drop candidates that are not on the correct column
    auto start_row = static_cast<int32_t>(move_str[0] - '1');
    candidates.erase(rs::remove_if(candidates,
                                   [start_row](Coordinates piece_loc)
                                   {
                                     return piece_loc.y() != start_row;
                                   })
                       .begin(),
                     candidates.end());
    move_str = move_str.substr(1);
    if (candidates.size() == 1)
    {
      // Exactly one piece can move to the target square

      auto const is_ep = is_en_passant(piece, candidates.front(), *target_square, *this);
      auto const victim = is_ep ? Piece::pawn : get_piece(*target_square);
      auto const move_type = is_ep ? Move_type::en_passant : Move_type::normal;
      return Move{candidates.front(), *target_square, piece, victim, promotion_result, move_type};
    }
  }

  if (candidates.size() > 1)
  {
    return tl::unexpected(std::string{"Too many pieces can perform move"});
  }
  return tl::unexpected(std::string{"No piece can perform move"});
}

tl::expected<Board, std::string> Board::from_pgn(std::string_view pgn)
{
  std::vector<std::string> tag_pairs;
  std::vector<std::string> moves;

  size_t index{0};
  std::string game_result;

  while (index < pgn.size() && index != std::string::npos)
  {
    // Skip to the next non-space character, starting at the first character we
    // haven't seen yet
    if (index != 0)
    {
      ++index;
    }
    index = pgn.find_first_not_of(" \t\n", index);

    if (pgn[index] == '[')
    {
      auto end_index = pgn.find(']', index);
      tag_pairs.emplace_back(pgn.substr(index + 1, end_index - index - 1)); // Drop the [] in the tag pair
      index = end_index;
    }
    else if (isalpha(pgn[index]) || pgn[index] == '0') // Castling uses both 'O and '0'
    {
      auto end_index = pgn.find_first_of(" \t\n", index);
      moves.emplace_back(pgn.substr(index, end_index - index));
      index = end_index;
    }
    else if (isdigit(pgn[index])) // Skip move number
    {
      auto end_index = pgn.find('.', index);
      if (end_index == std::string::npos)
      {
        game_result = pgn.substr(index, end_index - index);
      }
      index = end_index;
    }
    else if (pgn[index] == ';') // Skip comments
    {
      index = pgn.find('\n', index);
    }
    else if (pgn[index] == '{') // Skip comments
    {
      index = pgn.find('}', index);
    }
    else
    {
      std::stringstream err_ss;
      err_ss << "Unexpected char at index " << std::to_string(index) << " while parsing pgn file: " << pgn[index]
             << "Ascii code: " << std::to_string(static_cast<int32_t>(pgn[index])) << std::endl;
      return tl::unexpected(err_ss.str());
    }
  }

  // Play out the target moves on the board, and ensure they are valid
  Board result{};
  for (auto const& move_str : moves)
  {
    auto attempt = result.try_move_algebraic(move_str);
    if (!attempt)
    {
      return tl::unexpected(attempt.error());
    }
  }

  return tl::expected<Board, std::string>{std::move(result)};
}

bool Board::update_castling_rights_fen_(char c)
{
  switch (c)
  {
    case 'k':
      set_black_short_castle_true(m_rights);
      break;
    case 'q':
      set_black_long_castle_true(m_rights);
      break;
    case 'K':
      set_white_short_castle_true(m_rights);
      break;
    case 'Q':
      set_white_long_castle_true(m_rights);
      break;
    default:
      return false;
  }
  return true;
}

std::string Board::castling_rights_to_fen_() const
{
  std::stringstream ss;
  if (white_can_short_castle(m_rights))
  {
    ss << 'K';
  }
  if (white_can_long_castle(m_rights))
  {
    ss << 'Q';
  }
  if (black_can_short_castle(m_rights))
  {
    ss << 'k';
  }
  if (black_can_long_castle(m_rights))
  {
    ss << 'q';
  }

  auto result = ss.str();
  if (result.empty())
  {
    return "-";
  }
  return result;
}

tl::expected<Board, std::string> Board::from_fen(std::string_view fen)
{
  tl::expected<Board, std::string> board{Board{0}};

  std::string fen_str{fen.substr(fen.find_first_not_of(" \n\t"))};

  int8_t y{c_board_dimension - 1};
  int8_t x{0};

  size_t index{0};
  while (index < fen_str.size() && (fen_str[index] != ' ') && y >= 0)
  {
    if (isdigit(fen_str[index]))
    {
      for (uint8_t i{0}; i < (fen_str[index] - '0'); ++i)
      {
        x = (x + 1) % c_board_dimension;
      }
    }
    else if (isalpha(fen_str[index]))
    {
      auto color = islower(fen_str[index]) ? Color::black : Color::white;
      auto piece = from_char(std::toupper(fen_str[index], std::locale()));
      board->add_piece_(color, piece, {x, y});

      x = (x + 1) % c_board_dimension;
    }
    else if (fen_str[index] == '/')
    {
      if (x != 0)
      {
        return tl::unexpected(std::string{"Badly formed fen string - each row must have exactly eight squares"});
      }
      --y;
    }

    ++index;
  }

  if (fen_str[index] != ' ')
  {
    return tl::unexpected(std::string{"Badly formed fen string - Too many piece locations"});
  }
  ++index;

  if (std::tolower(fen_str[index], std::locale()) == 'w')
  {
    board->m_active_color = Color::white;
  }
  else if (std::tolower(fen_str[index], std::locale()) == 'b')
  {
    board->m_active_color = Color::black;
  }
  else
  {
    return tl::unexpected(std::string{"Badly formed fen string - expected current move color"});
  }

  ++index;
  if (fen_str[index] != ' ')
  {
    return tl::unexpected(std::string{"Badly formed fen string - expected space after current move"});
  }

  ++index;
  while (fen_str[index] != ' ')
  {
    board->update_castling_rights_fen_(fen_str[index]);
    ++index;
  }
  ++index;

  if (fen_str[index] != '-')
  {
    // En passant is possible, denote this by setting previous move to the pawn
    // move that allowed en passant

    auto capture_square = Coordinates::from_str(std::string_view{fen_str.c_str() + index, 2});
    if (!capture_square)
    {
      return tl::unexpected(std::string{"Badly formed fen string - invalid en passant capture square"});
    }

    board->m_en_passant_square.set_square(*capture_square);
    ++index;
  }

  ++index;
  if (index < fen_str.size() && fen_str[index] != ' ')
  {
    return tl::unexpected(std::string{"Badly formed fen string - expected space after en passant square"});
  }

  while (index < fen_str.size() && fen_str[index] == ' ')
  {
    ++index;
  }

  // Full move and ply counts are optional
  if (index < fen_str.size())
  {
    size_t last_char{0};
    board->m_halfmove_clock = static_cast<uint8_t>(std::stoi(fen_str.substr(index), &last_char));

    index += last_char;
    if (index >= fen_str.size())
    {
      return tl::unexpected(std::string{"Badly formed fen string - expected full move count"});
    }
    board->m_fullmove_count = static_cast<uint8_t>(std::stoi(fen_str.substr(index), &last_char));
  }

  board->m_zhash = {*board};
  MY_ASSERT(board->validate_(), "Invalid board created during fen parsing");

  return board;
}

std::string Board::to_fen() const
{
  std::stringstream result;

  auto to_char = [](Piece piece, Color color) -> char
  {
    std::stringstream ss;
    ss << piece;

    char result = ss.str().front();
    return (color == Color::white) ? std::toupper(result, std::locale()) : std::tolower(result, std::locale());
  };

  for (int8_t y = c_board_dimension - 1; y >= 0; --y)
  {
    int empty_counter{0};
    for (int8_t x = 0; x < c_board_dimension; ++x)
    {
      Coordinates sq{x, y};
      if (is_occupied(sq))
      {
        if (empty_counter > 0)
        {
          result << std::to_string(empty_counter);
          empty_counter = 0;
        }
        result << to_char(get_piece(sq), get_piece_color(sq));
      }
      else
      {
        ++empty_counter;
      }
    }

    if (empty_counter > 0)
    {
      result << std::to_string(empty_counter);
    }
    if (y > 0)
    {
      result << '/';
    }
  }
  result << ' ';
  result << ((get_active_color() == Color::white) ? 'w' : 'b');
  result << ' ';

  result << castling_rights_to_fen_();
  result << ' ';

  if (!get_en_passant_square().is_empty())
  {
    Coordinates en_passant_square{get_en_passant_square().bitscan_forward()};
    result << en_passant_square;
  }
  else
  {
    result << '-';
  }

  result << " " << std::to_string(m_halfmove_clock) << " " << std::to_string(m_fullmove_count);

  return result.str();
}

void Board::set_use_unicode_output(bool value)
{
  s_use_unicode_output = value;
}

bool Board::get_use_unicode_output()
{
  return s_use_unicode_output;
}

std::string square_str(Coordinates location, Board const& board)
{
  auto const piece = board.get_piece(location);

  std::stringstream ss;
  ss << piece;

  ss << "_";

  if (piece != Piece::empty)
  {
    switch (board.get_piece_color(location))
    {
      case Color::black:
        ss << 'b';
        break;
      case Color::white:
        ss << 'w';
        break;
      default:
        MY_ASSERT(false, "Square occupier should be black or white");
        break;
    }
  }
  else
  {
    ss << "_";
  }

  ss << " ";
  return ss.str();
}

auto piece_to_unicode_symbol(Color color, Piece piece)
{
  if (color == Color::black)
  {
    switch (piece)
    {
      case Piece::pawn:
        return "♟︎";
      case Piece::knight:
        return "♞";
      case Piece::bishop:
        return "♝";
      case Piece::rook:
        return "♜";
      case Piece::queen:
        return "♛";
      case Piece::king:
        return "♚";
      default:
        MY_ASSERT(false, "Invalid piece");
        return "_";
    }
  }

  switch (piece)
  {
    case Piece::pawn:
      return "♙";
    case Piece::knight:
      return "♘";
    case Piece::bishop:
      return "♗";
    case Piece::rook:
      return "♖";
    case Piece::queen:
      return "♕";
    case Piece::king:
      return "♔";
    default:
      MY_ASSERT(false, "Invalid piece");
      return "_";
  }
}

std::ostream& operator<<(std::ostream& out, Board const& self)
{
  if (Board::get_use_unicode_output())
  {
    out << "\n";
    for (int i = c_board_dimension - 1; i >= 0; i--)
    {
      out << (i + 1) << "  "; // Print rank
      for (int j = 0; j < c_board_dimension; j++)
      {
        auto location = Coordinates{j, i};
        auto piece = self.get_piece(location);
        out << " " << ((piece == Piece::empty) ? "." : piece_to_unicode_symbol(self.get_piece_color(location), piece))
            << " ";
      }
      out << "\n\n";
    }

    out << "   ";

    // Print files
    for (int i = 0; i < c_board_dimension; i++)
    {
      out << " " << static_cast<char>(i + 'A') << " ";
    }
    out << "\n";
  }
  else
  {
    out << "\n";
    for (int i = c_board_dimension - 1; i >= 0; i--)
    {
      out << (i + 1) << "  "; // Print rank
      for (int j = 0; j < c_board_dimension; j++)
      {
        out << square_str({j, i}, self);
      }
      out << "\n\n";
    }

    // Print files
    out << "   ";
    for (int i = 0; i < c_board_dimension; i++)
    {
      out << " " << static_cast<char>(i + 'A') << "  ";
    }
    out << "\n";
  }

  return out;
}
} // namespace Meneldor
