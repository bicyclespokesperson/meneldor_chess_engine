#include <catch2/catch.hpp>

#include "board.h"
#include "move_generator.h"

namespace
{

void test_perft(std::string_view fen, int depth, uint64_t expected)
{
  auto board = *Board::from_fen(fen);

  std::atomic_flag is_cancelled{false};
  uint64_t actual = Move_generator::perft(depth, board, is_cancelled);

  REQUIRE(actual == expected);
}

} // namespace

// Perft results source: https://www.chessprogramming.org/Perft_Results

TEST_CASE("Perft position 1", "[Move_generator]")
{
  Board board;
  std::atomic_flag is_cancelled{false};
  uint64_t actual = Move_generator::perft(4, board, is_cancelled);

  uint64_t expected{197'281};
  REQUIRE(actual == expected);
}

TEST_CASE("Perft position 2", "[.Move_generator]")
{
  std::string fen_str = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ";
  int depth{4};
  uint64_t expected{4'085'603};
  test_perft(fen_str, depth, expected);
}

TEST_CASE("Perft position 3", "[.Move_generator]")
{
  std::string fen_str = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ";
  int depth{5};
  uint64_t expected{674'624};
  test_perft(fen_str, depth, expected);
}

TEST_CASE("Perft position 4", "[.Move_generator]")
{
  std::string fen_str = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
  int depth{4};
  uint64_t expected{422'333};

  test_perft(fen_str, depth, expected);
}

TEST_CASE("Perft position 5", "[.Move_generator]")
{
  std::string fen_str = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8";
  int depth{4};
  uint64_t expected{2'103'487};
  test_perft(fen_str, depth, expected);
}

TEST_CASE("Perft position 6", "[.Move_generator]")
{
  std::string fen_str = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10";
  int depth{4};
  uint64_t expected{3'894'594};
  test_perft(fen_str, depth, expected);
}
