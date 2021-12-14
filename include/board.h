#ifndef BOARD_H
#define BOARD_H

#include "bitboard.h"
#include "castling_rights.h"
#include "coordinates.h"
#include "move.h"
#include "zobrist_hash.h"

class Board
{
public:
  static std::optional<Board> from_pgn(std::string_view pgn);

  static std::optional<Board> from_fen(std::string_view fen);

  static void set_use_unicode_output(bool value);

  static bool get_use_unicode_output();

  Board();

  Board(Board const& other) = default;
  Board(Board&& other) = default;
  Board& operator=(Board const& other) = default;
  Board& operator=(Board&& other) = default;

  ~Board() = default;

  std::string to_fen() const;

  /**
   * Checks if every square in between squares from and to are empty,
   * and if the squares are in a vertical line.
   * @param from The initial square
   * @param to The final square
   * @return True if there is a clear vertical line between the squares
   */
  bool is_clear_vertical(Coordinates from, Coordinates to) const;

  /**
   * Checks if every square in between squares from and to are empty,
   * and if the squares are in a horizontal line.
   * @param from The initial square
   * @param to The final square
   * @return True if there is a clear horizontal line between the squares
   */
  bool is_clear_horizontal(Coordinates from, Coordinates to) const;

  /**
   * Checks if every square in between squares from and to are empty,
   * and if the squares are in a diagonal line.
   * @param from The initial square
   * @param to The final square
   * @return True if there is a clear diagonal line between the squares
   */
  bool is_clear_diagonal(Coordinates from, Coordinates to) const;

  /**
   * Returns the distance between two squares on the board.
   * For example, A2 and A5 are three squares apart
   * Returns -1 if the squares are not in a line on the board.
   * @param from The first Square
   * @param to The Second Square
   * @return The distance between the two squares.
   */
  int distance_between(Coordinates from, Coordinates to) const;

  /**
   * Initializes the board with pieces in their standard positions
   */
  void reset();

  /**
   * Tries to make a move
   *
   * Returns an empty optional for an illegal move, Piece::empty if the move succeeded
   * but no piece was captured, and the piece if a piece was captured.
   */
  bool try_move(Move m);

  /**
   * Performs a move without validating the move
   * @param skip_check_detection If this is false, the method will make sure the king is not in check
   */
  bool move_no_verify(Move m, bool skip_check_detection = true);

  /**
   * A fast try move method that will return true if the move results in check.
   * Does not perform verification on whether or not the move is legal.
   * Leaves the board in an invalid state.
   */
  bool move_results_in_check_destructive(Move m);

  /**
   * Captured piece can be Piece::empty to signify no capture was performed
   */
  bool undo_move(Move m, Bitboard en_passant_square, Castling_rights rights, uint8_t halfmove_clock);

  /**
   * Attempt to make a move encoded in uci format ("e2 e4")
   *
   * Returns an empty optional for an illegal move or invalid string, Piece::empty if the move succeeded
   * but no piece was captured, and the piece if a piece was captured.
   */
  bool try_move_uci(std::string_view move_str);

  /**
   * Attempt to make a move encoded in algebraic notation ("Bxc3")
   *
   * Returns an empty optional for an illegal move or invalid string, Piece::empty if the move succeeded
   * but no piece was captured, and the piece if a piece was captured.
   */
  bool try_move_algebraic(std::string_view move_str);

  std::optional<Move> move_from_algebraic(std::string_view move_param, Color color) const;

  std::optional<Move> move_from_uci(std::string move_str) const;

  /**
   * Check if castling to the desired square is allowed
   * (the pieces involved haven't moved)
   */
  bool can_castle_to(Coordinates dest) const;

  Castling_rights get_castling_rights() const;

  std::vector<Coordinates> find_pieces_that_can_move_to(Piece piece, Color color, Coordinates target_square) const;

  Color get_active_color() const;

  bool is_in_check(Color color) const;

  Game_state calc_game_state() const;

  Bitboard get_piece_set(Color color, Piece piece) const;

  Bitboard get_all(Piece piece) const;

  Bitboard get_all(Color color) const;

  Bitboard get_en_passant_square() const;

  Bitboard get_occupied_squares() const;

  bool is_occupied(Coordinates square) const;

  Color get_piece_color(Coordinates square) const;

  Piece get_piece(Coordinates square) const;

  uint8_t get_halfmove_clock() const;

  uint8_t get_move_count() const;

  // Returns true if color has enough material to win the game
  bool has_sufficient_material(Color color) const;

  Zobrist_hash get_hash_key() const;

private:
  /**
   * Private constructor that doesn't initialize pieces
   */
  Board(int);

  /**
   * Ensure that the piece sets are internally consistent
   */
  bool validate_() const;

  void update_castling_rights_(Color color, Piece piece, Move m);
  bool update_castling_rights_fen_(char c);
  std::string castling_rights_to_fen_() const;

  std::optional<std::pair<Coordinates, Piece>> perform_move_(Move m, Coordinates capture_location);
  void unperform_move_(Color color, Move m, std::optional<std::pair<Coordinates, Piece>> captured_piece);
  Move find_castling_rook_move_(Coordinates king_destination) const;

  void add_piece_(Color color, Piece piece, Coordinates to_add);
  void remove_piece_(Color color, Piece piece, Coordinates to_remove);

  std::array<Bitboard, static_cast<uint8_t>(Piece::_count)> m_bitboards;
  Bitboard m_en_passant_square{0};
  Zobrist_hash m_zhash;
  Castling_rights m_rights{c_castling_rights_none};
  Color m_active_color{Color::white};
  uint8_t m_halfmove_clock{0};
  uint8_t m_fullmove_count{1};

  //NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables) Only used internally by board
  static inline bool s_use_unicode_output{false};
};

std::ostream& operator<<(std::ostream& out, Board const& self);

#endif
