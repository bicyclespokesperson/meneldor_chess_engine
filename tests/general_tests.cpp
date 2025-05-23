#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include "bitboard.h"
#include "board.h"
#include "meneldor_engine.h"
#include "move_generator.h"
#include "transposition_table.h"
#include "utils.h"
#include "zobrist_hash.h"

namespace rs = std::ranges;
namespace Meneldor
{
std::optional<std::string> read_file_contents(std::string const& filename)
{
  std::ifstream infile{filename};
  if (!infile)
  {
    return {};
  }
  return std::string{std::istreambuf_iterator<char>(infile), std::istreambuf_iterator<char>()};
}

static std::string const c_fischer_spassky_result = R"(
8  ___ ___ ___ ___ ___ ___ ___ ___ 

7  ___ ___ ___ ___ ___ ___ ___ ___ 

6  ___ ___ ___ ___ R_w ___ P_b ___ 

5  ___ ___ K_b ___ ___ ___ P_b ___ 

4  ___ P_b ___ ___ ___ ___ P_w ___ 

3  ___ P_w ___ B_b ___ P_w ___ ___ 

2  ___ ___ ___ K_w ___ N_b ___ ___ 

1  ___ ___ ___ ___ ___ ___ ___ ___ 

    A   B   C   D   E   F   G   H  
)";

static std::string const c_sigrist_result = R"(
8  R_b ___ ___ ___ ___ ___ ___ ___ 

7  P_b P_b P_b ___ ___ ___ ___ P_b 

6  ___ ___ ___ ___ B_w ___ ___ K_b 

5  ___ ___ ___ ___ ___ ___ ___ ___ 

4  ___ ___ ___ P_w ___ Q_w ___ ___ 

3  ___ ___ N_w ___ N_b ___ ___ ___ 

2  P_w P_w P_w ___ ___ ___ P_w P_w 

1  R_w ___ ___ ___ ___ ___ K_w ___ 

    A   B   C   D   E   F   G   H  
)";

static std::string const c_fen_test_result = R"(
8  R_b ___ ___ ___ K_b ___ ___ R_b 

7  Q_b P_b P_b B_b ___ P_b P_b ___ 

6  ___ ___ N_b B_b P_b N_b ___ ___ 

5  ___ B_w ___ N_w ___ ___ ___ ___ 

4  P_b P_w ___ P_w P_w ___ Q_b P_w 

3  P_w ___ P_w ___ ___ ___ N_w ___ 

2  ___ ___ ___ B_w Q_w P_w ___ ___ 

1  R_w ___ ___ ___ K_w ___ ___ R_w 

    A   B   C   D   E   F   G   H  
)";

static std::string const c_uci_moves_result = R"(
8  R_b N_b B_b Q_b K_b B_b ___ R_b 

7  P_b P_b P_b P_b P_b P_b P_b P_b 

6  ___ ___ ___ ___ ___ N_b ___ ___ 

5  P_w ___ ___ ___ ___ ___ ___ ___ 

4  ___ ___ ___ ___ ___ ___ ___ ___ 

3  ___ ___ ___ ___ ___ ___ ___ ___ 

2  ___ P_w P_w P_w P_w P_w P_w P_w 

1  R_w N_w B_w Q_w K_w B_w N_w R_w 

    A   B   C   D   E   F   G   H  
)";

TEST_CASE("A board can be constructed from a pgn file", "[board]")
{
  std::string const pgn_filename{"./data/fischer_spassky.pgn"};
  auto contents = read_file_contents(pgn_filename);
  REQUIRE(contents.has_value());

  auto board = Board::from_pgn(*contents);

  REQUIRE(board.has_value());

  std::stringstream out;
  out << *board;

  std::string result = out.str();

  // Compare ignoring whitespace
  REQUIRE(rs::equal(result | rs::views::filter(std::not_fn(isspace)),
                    c_fischer_spassky_result | rs::views::filter(std::not_fn(isspace))));
}

TEST_CASE("A pgn file from chess.com", "[board]")
{
  std::string const pgn_filename{"./data/sigrist.pgn"};
  auto contents = read_file_contents(pgn_filename);
  REQUIRE(contents.has_value());

  auto board = Board::from_pgn(*contents);
  REQUIRE(board.has_value());

  std::stringstream out;
  out << *board;

  std::string result = out.str();
  std::string expected = c_sigrist_result;

  // Compare ignoring whitespace
  REQUIRE(rs::equal(result | rs::views::filter(std::not_fn(isspace)),
                    c_sigrist_result | rs::views::filter(std::not_fn(isspace))));
}

TEST_CASE("A board should be initially setup", "[board]")
{
  Board board;

  // Sampling of pieces to ensure the board is setup initially
  REQUIRE(board.get_piece({0, 0}) == Piece::rook);
  REQUIRE(board.get_piece_color({0, 0}) == Color::white);
  REQUIRE(board.get_piece({4, 0}) == Piece::king);
  REQUIRE(board.get_piece_color({4, 0}) == Color::white);
  REQUIRE(board.get_piece({3, 0}) == Piece::queen);
  REQUIRE(board.get_piece_color({3, 0}) == Color::white);

  REQUIRE(board.get_piece({3, 3}) == Piece::empty);

  REQUIRE(board.get_piece({6, 6}) == Piece::pawn);
  REQUIRE(board.get_piece_color({6, 6}) == Color::black);
  REQUIRE(board.get_piece({6, 7}) == Piece::knight);
  REQUIRE(board.get_piece_color({6, 7}) == Color::black);
  REQUIRE(board.get_piece({5, 7}) == Piece::bishop);
  REQUIRE(board.get_piece_color({5, 7}) == Color::black);
}

TEST_CASE("A board can be created from a FEN string", "[board]")
{
  static std::string const fen_string{"r3k2r/qppb1pp1/2nbpn2/1B1N4/pP1PP1qP/P1P3N1/3BQP2/R3K2R b Qk b3 0 19"};

  auto board = Board::from_fen(fen_string);

  std::stringstream out;
  out << *board;

  std::string result = out.str();
  REQUIRE(rs::equal(result | rs::views::filter(std::not_fn(isspace)),
                    c_fen_test_result | rs::views::filter(std::not_fn(isspace))));

  REQUIRE(!board->try_move_algebraic("O-O-O")); // Should fail as black does not have queenside castling rights
  REQUIRE(board->try_move_algebraic("axb3")); // En passant should succeed
  REQUIRE(!board->try_move_algebraic("O-O")); // Should fail as white does not have kingside castling rights
  REQUIRE(board->try_move_algebraic("O-O-O"));
  REQUIRE(board->try_move_algebraic("O-O"));
}

TEST_CASE("Fen round trip", "[board]")
{
  static std::string const fen{"r1b4r/p1kppPP1/2p5/1pP5/1B2N3/KP6/P2P2pp/R2Q4 w - - 0 1"};
  auto board1 = Board::from_fen(fen);

  std::string generated_fen = board1->to_fen();
  auto board2 = Board::from_fen(generated_fen);

  REQUIRE(generated_fen == fen);

  std::stringstream first;
  first << *board1;
  std::stringstream second;
  second << *board2;

  REQUIRE(first.str() == second.str());
}

TEST_CASE("Fen castling rights and en passant"
          "[board]")
{
  static std::string const fen{"r3k2r/qppb1pp1/2nbpn2/1B1N4/pP1PP1qP/P1P3N1/3BQP2/R3K2R b Qk b3 0 1"};
  auto board = Board::from_fen(fen);

  std::string generated_fen = board->to_fen();
  REQUIRE(generated_fen == fen);
}

TEST_CASE("A board should prevent illegal moves", "[board]")
{
  Board board;

  {
    auto result = board.try_move_algebraic("d5"); // Black cannot move first
    REQUIRE(!result);
    REQUIRE(result.error() == "No piece can perform move");
  }

  REQUIRE(!board.try_move_algebraic("Bc4")); // Bishops cannot jump over pieces
  REQUIRE(board.try_move_algebraic("e4"));

  REQUIRE(board.try_move_algebraic("e6"));

  REQUIRE(!board.try_move_algebraic("e3")); // Pawns cannot move backwards
  REQUIRE(!board.try_move_algebraic("exf5")); // Pawns cannot capture to an empty square
  REQUIRE(board.try_move_algebraic("e5"));

  REQUIRE(board.try_move_algebraic("f5"));

  REQUIRE(!board.try_move_algebraic("e6")); // Pawns cannot capture forward
  REQUIRE(!board.try_move_algebraic("exd6")); // Test en passant
  REQUIRE(board.try_move_algebraic("exf6"));

  REQUIRE(board.try_move_algebraic("a6"));

  REQUIRE(board.try_move_algebraic("f7+"));

  {
    auto result = board.try_move_algebraic("Nc6"); // A move cannot leave the king in check
    REQUIRE(!result);
    REQUIRE(result.error() == "Move leaves king in check"); // A move cannot leave the king in check
  }

  REQUIRE(board.try_move_algebraic("Kxf7"));

  REQUIRE(board.try_move_algebraic("Qf3+"));

  REQUIRE(board.try_move_algebraic("Nf6")); // Block the check

  REQUIRE(board.try_move_algebraic("Nh3"));

  REQUIRE(!board.try_move_algebraic("Ng4")); // A move cannot leave the king in check
  REQUIRE(board.try_move_algebraic("d5"));

  REQUIRE(board.try_move_algebraic("Be2"));

  REQUIRE(!board.try_move_algebraic("a4")); // Pawns cannot move two spaces besides their first move
  REQUIRE(board.try_move_algebraic("Be7")); // Pawns cannot move two spaces besides their first move

  REQUIRE(board.try_move_algebraic("O-O"));

  REQUIRE(board.try_move_algebraic("Ke8"));

  REQUIRE(!board.try_move_algebraic("Qg5")); // Queens cannot move like Knights
  REQUIRE(board.try_move_algebraic("Qg3"));

  REQUIRE(!board.try_move_algebraic("O-O")); // Cannot castle after the king has already moved
  REQUIRE(board.try_move_algebraic("Rf8"));

  REQUIRE(board.try_move_algebraic("Bb5"));

  REQUIRE(board.try_move_algebraic("xb5"));
}

TEST_CASE("Castling rights", "[board]")
{
  Board board;
  REQUIRE(board.try_move_algebraic("h4"));
  REQUIRE(board.try_move_algebraic("a5"));

  REQUIRE(board.try_move_algebraic("Rh3"));
  REQUIRE(board.try_move_algebraic("Ra7"));

  REQUIRE(board.try_move_algebraic("g3"));
  REQUIRE(board.try_move_algebraic("b6"));

  REQUIRE(board.try_move_algebraic("Bg2"));
  REQUIRE(board.try_move_algebraic("Rb7"));

  REQUIRE(board.try_move_algebraic("Nf3"));
  REQUIRE(board.try_move_algebraic("Nc6"));

  REQUIRE(!board.try_move_algebraic("O-O")); // Cannot castle kingside if the rook has already moved
  REQUIRE(board.try_move_algebraic("a3"));

  REQUIRE(!board.try_move_algebraic("O-O-O")); // Cannot castle queenside if the rook has already moved
}

TEST_CASE("En passant move type", "[board]")
{
  auto board = *Board::from_fen("r2k1b2/p4Bp1/bp3p1r/4q3/1n1Pp3/1Q2P1Pp/P1P2P1P/R1B1NRK1 b - d3 0 21");
  auto m = board.move_from_uci("e4d3");
  REQUIRE(m->type() == Move_type::en_passant);

  m = board.move_from_uci("b6b5");
  REQUIRE(m->type() == Move_type::normal);
}

TEST_CASE("A board can make moves in uci format", "[board]")
{
  Board board;
  REQUIRE(board.try_move_uci("a2 a4"));
  REQUIRE(!board.try_move_uci("a4 a5"));
  REQUIRE(board.try_move_uci("g8 f6"));
  REQUIRE(board.try_move_uci("a4a5"));

  std::stringstream out;
  out << board;

  std::string expected{c_uci_moves_result};
  REQUIRE(expected == out.str());
}

TEST_CASE("Pawn promotion", "[board]")
{
  static std::string const fen{"r1b4r/p1kppPP1/2p5/1pP5/1B2N3/KP6/P2P2pp/R2Q4 w - - 0 1"};
  auto board = Board::from_fen(fen);
  REQUIRE(board.has_value()); // Pawn promotion is invalid without a specified
    // promotion piece

  REQUIRE(!board->try_move_uci("g7 g8")); // Pawn promotion is invalid without a
    // specified promotion piece
  REQUIRE(board->try_move_uci("g7 g8q"));
  REQUIRE(!board->try_move_uci("h2h1k")); // Cannot promote to king
  REQUIRE(!board->try_move_uci("h2h1p")); // Cannot promote to pawn
  REQUIRE(board->try_move_uci("h2h1n"));
  REQUIRE(board->try_move_algebraic("f8=R"));
  REQUIRE(board->try_move_algebraic("g1=B"));
}

TEST_CASE("Check for checkmate", "[board]")
{
  Board board;
  board.try_move_algebraic("e4");
  board.try_move_algebraic("e5");
  board.try_move_algebraic("Bc4");
  board.try_move_algebraic("Nc6");
  board.try_move_algebraic("Qh5");
  REQUIRE(board.calc_game_state() == Game_state::in_progress);
  board.try_move_algebraic("Nf6");
  REQUIRE(board.calc_game_state() == Game_state::in_progress);
  board.try_move_algebraic("Qxf7#");

  REQUIRE(board.calc_game_state() == Game_state::white_victory);

  board = *Board::from_fen("rnb1kbnr/p2ppppp/2p5/qp6/3PP3/1B6/PPP2PPP/RNBQK1NR w KQkq - 2 5");
  REQUIRE(board.calc_game_state() == Game_state::in_progress);

  board = *Board::from_fen("rn2k1nr/pp4pp/3p4/b1pP4/P1P2p1q/1b2pPRP/1P1NP1PQ/2B1KBNR w Kkq - 0 13");
  REQUIRE(board.calc_game_state() == Game_state::draw);
}

TEST_CASE("bitboard", "[bitboard]")
{
  Bitboard b{0};
  b.set_square(9);
  b.set_square(63);

  Bitboard b2{0};
  b2.set_square({1, 1});
  b2.set_square({7, 7});

  REQUIRE(b.val == b2.val);
  REQUIRE(b.occupancy() == 2);

  Bitboard b3{0x0ff0};
  Bitboard b4{0x00ff};
  REQUIRE((b3 & b4).val == 0x00f0);
  REQUIRE((b3 | b4).val == 0x0fff);
  REQUIRE((~b3).val == ~(b3.val));
}

TEST_CASE("Bitboard iterator", "[bitboard]")
{
  Bitboard b;
  b.set_square(1);
  b.set_square(2);
  b.set_square(4);
  b.set_square(9);
  b.set_square(63);
  auto total = std::accumulate(b.begin(), b.end(), 0);

  REQUIRE(total == 79);
}

TEST_CASE("Undo move", "[board]")
{
  static std::string const fen_string{"r3k2r/qppb1pp1/2nbpn2/1B1N4/pP1PP1qP/P1P3N1/3BQP2/R3K2R b Qk b3 7 9"};
  auto board = Board::from_fen(fen_string);
  auto color = board->get_active_color();

  // Save state
  auto en_passant_square = board->get_en_passant_square();
  auto castling_rights = board->get_castling_rights();
  auto halfmove_clock = board->get_halfmove_clock();
  auto m = board->move_from_algebraic("axb3", color);
  REQUIRE(board->try_move(*m));
  REQUIRE(m->type() == Move_type::en_passant);

  board->undo_move(*m, en_passant_square, castling_rights, halfmove_clock);

  REQUIRE(board->to_fen() == fen_string);
}

TEST_CASE("Undo move 2", "[board]")
{
  static std::string const fen_string{"rn1qk2r/pbpp1ppp/1p2pn2/8/3P4/b1NQB2P/PPP1PPP1/R3KBNR w KQkq - 9 15"};
  auto board = Board::from_fen(fen_string);

  // Save state
  auto en_passant_square = board->get_en_passant_square();
  auto castling_rights = board->get_castling_rights();
  auto halfmove_clock = board->get_halfmove_clock();

  auto m = board->move_from_uci("e1c1");
  REQUIRE(m->type() == Move_type::normal);
  REQUIRE(board->move_no_verify(*m));

  board->undo_move(*m, en_passant_square, castling_rights, halfmove_clock);

  REQUIRE(board->to_fen() == fen_string);
}

TEST_CASE("Starting moves", "[Move_generator]")
{
  Board board;
  auto moves = Move_generator::generate_legal_moves(board);
  REQUIRE(moves.size() == 20);
}

TEST_CASE("Test is_square_attacked", "[Move_generator]")
{
  Board board;
  Move_generator::generate_legal_moves(board);

  Bitboard bb;
  bb.set_square(a3);
  REQUIRE(!Move_generator::is_square_attacked(board, Color::black, bb));

  bb.unset_all();
  bb.set_square(a3);
  REQUIRE(Move_generator::is_square_attacked(board, Color::white, bb));

  bb.unset_all();
  bb.set_square(e6);
  REQUIRE(Move_generator::is_square_attacked(board, Color::black, bb));

  bb.unset_all();
  bb.set_square(e6);
  REQUIRE(!Move_generator::is_square_attacked(board, Color::white, bb));
}

TEST_CASE("Move counts", "[board]")
{
  Board board;
  REQUIRE(board.get_move_count() == 1);
  REQUIRE(board.get_halfmove_clock() == 0);

  board.try_move_algebraic("bxc5"); // Illegal move should not change move counts

  REQUIRE(board.get_move_count() == 1);
  REQUIRE(board.get_halfmove_clock() == 0);

  board.try_move_algebraic("e4");
  board.try_move_algebraic("e5");
  board.try_move_algebraic("Nf3");
  board.try_move_algebraic("Nf6");

  REQUIRE(board.get_move_count() == 3);
  REQUIRE(board.get_halfmove_clock() == 2);

  board.try_move_algebraic("Nxe5");
  board.try_move_algebraic("Nc6");

  REQUIRE(board.get_move_count() == 4);
  REQUIRE(board.get_halfmove_clock() == 1);
}

TEST_CASE("Threefold repetition", "[Threefold_repetition_detector]")
{
  Threefold_repetition_detector detector;
  REQUIRE(!detector.add_fen("r1bqk2r/p2p1pbp/1pn3p1/1p1Np2n/4PP2/P2P4/1PP1N1PP/R1B2RK1 b kq f3 1 1"));
  REQUIRE(!detector.add_fen("r1bqk2r/p2p1pbp/1pn3p1/1p1Np2n/4PP2/P2P4/1PP1N1PP/"
                            "R1B2RK1 b kq f3 30 10 "));
  REQUIRE(!detector.add_fen("r3k2r/qppb1pp1/2nbpn2/1B1N4/pP1PP1qP/P1P3N1/3BQP2/R3K2R b Qk b3 0 19"));
  REQUIRE(detector.add_fen("r1bqk2r/p2p1pbp/1pn3p1/1p1Np2n/4PP2/P2P4/1PP1N1PP/"
                           "R1B2RK1 b kq f3 10 1 "));
}

TEST_CASE("Zobrist hashing", "[Zobrist_hash]")
{
  static std::string const fen_string{"r1bqk2r/p2p1pbp/1pn3p1/1p1Np2n/4PP2/P2P4/1PP1N1PP/R1B2RK1 b kq f3 1 1"};
  auto board = *Board::from_fen(fen_string);

  auto hash1 = Zobrist_hash(board);

  SECTION("White to play should change hash")
  {
    // Ensure that white to play yields a different hash
    static std::string const fen_string2{"r1bqk2r/p2p1pbp/1pn3p1/1p1Np2n/4PP2/"
                                         "P2P4/1PP1N1PP/R1B2RK1 w kq f3 1 1"};
    auto board2 = *Board::from_fen(fen_string2);
    auto hash2 = Zobrist_hash(board2);
    REQUIRE(hash1 != hash2);
    hash2.update_player_to_move();
    REQUIRE(hash1 == hash2);
  }

  SECTION("Different en passant square should change hash")
  {
    static std::string const fen_string2{"r1bqk2r/p2p1pbp/1pn3p1/1p1Np2n/4PP2/P2P4/1PP1N1PP/R1B2RK1 b kq - 1 1"};
    auto board2 = *Board::from_fen(fen_string2);
    auto hash2 = Zobrist_hash(board2);
    REQUIRE(hash1 != hash2);
  }

  SECTION("Different castling rights should change hash")
  {
    static std::string const fen_string2{"r1bqk2r/p2p1pbp/1pn3p1/1p1Np2n/4PP2/"
                                         "P2P4/1PP1N1PP/R1B2RK1 b kQ f3 1 1"};
    auto board2 = *Board::from_fen(fen_string2);
    auto hash2 = Zobrist_hash(board2);
    REQUIRE(hash1 != hash2);
    Castling_rights to_remove = board2.get_castling_rights();
    Castling_rights to_add = board.get_castling_rights();
    hash2.update_castling_rights(to_remove);
    hash2.update_castling_rights(to_add);
    REQUIRE(hash1 == hash2);
  }

  SECTION("Playing a move should change the hash")
  {
    // Require that playing a move changes the hash
    auto color = board.get_active_color();
    auto move = board.move_from_algebraic("Rb8", color);
    REQUIRE(move.has_value());
    auto old_hash = hash1;
    hash1.update_piece_location(color, Piece::rook, move->from());
    hash1.update_piece_location(color, Piece::rook, move->to());
    REQUIRE(hash1 != old_hash);
  }

  SECTION("The same board with piece colors swapped should change the hash")
  {
    static std::string const fen_string2{"r1bqk2r/p2p1pbp/1Pn3p1/1p1Np2n/4PP2/"
                                         "p2P4/1PP1N1PP/R1B2RK1 b kq f3 1 1"};
    auto board2 = *Board::from_fen(fen_string2);
    auto hash2 = Zobrist_hash(board2);
    REQUIRE(hash1 != hash2);
  }

  SECTION("The same board with piece colors swapped should change the hash")
  {
    static std::string const fen_string2{"r1bqk2r/p2p1pbp/1Pn3p1/1p1Np2n/4PP2/"
                                         "p2P4/1PP1N1PP/R1B2RK1 b kq f3 1 1"};
    auto board2 = *Board::from_fen(fen_string2);
    auto hash2 = Zobrist_hash(board2);
    REQUIRE(hash1 != hash2);
  }
}

TEST_CASE("Zobrist hash from fen should match hash from playing moves", "[Zobrist_hash]")
{
  std::string fen1 = "4Q3/p3B1pk/1p2p2p/2p4P/P1b1pP2/4n1K1/3r2P1/6NR w - - 0 31";
  std::string fen2 = "7k/p3B1p1/1p2p1Qp/2p4P/P1b1pP2/4n1K1/3r2P1/6NR w - - 2 32";

  auto board = *Board::from_fen(fen1);
  board.try_move_uci("e8g6");
  board.try_move_uci("h7h8");

  auto board2 = *Board::from_fen(fen2);
  REQUIRE(board.get_hash_key() == board2.get_hash_key());
}

TEST_CASE("Transposition table", "[Transposition_table]")
{
  Transposition_table tt{1024 * sizeof(Transposition_table::Entry)};

  Board board;
  board.try_move_algebraic("e4");
  board.try_move_algebraic("e5");
  board.try_move_algebraic("Nf3");
  board.try_move_algebraic("Nc6");
  board.try_move_algebraic("d4");
  board.try_move_algebraic("d6");

  auto m = board.move_from_uci("a2 a3");
  Transposition_table::Entry e{board.get_hash_key(), 2, 1, *m, Transposition_table::Eval_type::alpha};

  tt.insert(board.get_hash_key(), e);

  Board board2;

  board2.try_move_algebraic("d4");
  board2.try_move_algebraic("d6");

  board2.try_move_algebraic("e4");
  board2.try_move_algebraic("e5");

  board2.try_move_algebraic("Nf3");
  board2.try_move_algebraic("Nc6");

  REQUIRE(board.get_hash_key() == board2.get_hash_key());
  auto e2 = tt.get(board2.get_hash_key());
  REQUIRE(e2 != nullptr);
  REQUIRE(e2->evaluation == 1);
}

TEST_CASE("Coordinate constants are correct", "[Coordinates]")
{
  std::stringstream ss;
  ss << a1 << a7 << b1 << b7 << d5 << c7 << h8;
  REQUIRE(ss.str() == "a1a7b1b7d5c7h8");
}
} // namespace Meneldor
