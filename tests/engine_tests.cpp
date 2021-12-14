#include <catch2/catch.hpp>

#include "meneldor_engine.h"
#include "utils.h"

namespace
{
auto engine_stats_from_position(std::string_view fen, bool debug = false)
{
  static std::string const c_performance_log_filename{
    "/Users/jeremysigrist/Desktop/code_projects/chess_engine/output/performance_log.txt"};
  std::ofstream outfile{c_performance_log_filename, std::ios_base::app};
  MY_ASSERT(outfile.good(), "Outfile could not be opened");

  static bool first_call{true};
  if (first_call)
  {
    auto t = std::time(nullptr);
    auto current_time = std::localtime(&t);

    outfile << "\nTest run starting: " << std::put_time(current_time, "%m/%d/%Y at %H:%M:%S") << "\n";
    first_call = false;
  }

  Meneldor_engine engine;
  engine.setDebug(debug);
  engine.initialize();
  engine.setPosition(std::string{fen});

  senjo::GoParams params;
  params.depth = 7;
  params.nodes = 0; // ignored for now

  auto const engine_move = engine.go(params, nullptr);
  auto search_stats = engine.getSearchStats();
  auto elapsed_seconds = static_cast<double>(search_stats.msecs) / 1000.0;

  std::stringstream out;
  out << "For position: " << fen << "\n  Engine found " << engine_move << " after thinking for " << std::fixed
      << std::setprecision(2) << format_with_commas(elapsed_seconds) << " seconds and searching "
      << format_with_commas(search_stats.nodes) << " nodes ("
      << format_with_commas(search_stats.nodes / elapsed_seconds) << " nodes/sec)\n"
      << "  QNodes searched: " << format_with_commas(search_stats.qnodes) << "\n";

  std::cout << out.str();
  outfile << out.str();
}

} // namespace

TEST_CASE("Evaluate", "[Meneldor_engine]")
{
  std::string fen = "r1bqk2r/p2p1pbp/1pn3p1/1p1Np2n/4PP2/P2P4/1PP1N1PP/R1B2RK1 b kq f3 0 10";
  auto board = *Board::from_fen(fen);

  Meneldor_engine engine;

  // The evaluation function will change over time, but black is clearly winning in this position
  // Black to move -> should return a positive value to indicate black is better
  REQUIRE(engine.evaluate(board) > 0);
}

TEST_CASE("Search_opening", "[.Meneldor_engine]")
{
  std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
  engine_stats_from_position(fen);
}

TEST_CASE("Search_mid1", "[.Meneldor_engine]")
{
  // King's gambit accepted line
  std::string fen = "rnbqkbnr/pppp4/7p/8/2B1PppP/8/PPPP2P1/RNBQK2R w KQ - 2 8";
  engine_stats_from_position(fen);
}

TEST_CASE("Search_mid2", "[.Meneldor_engine]")
{
  // Vienna gambit main line
  std::string fen = "rn3rk1/pp2bp1p/6pQ/q2pPb2/2pP4/2P2N2/P1P1B1PP/R1B1K2R w KQ - 4 13";
  engine_stats_from_position(fen);
}

TEST_CASE("Search_mid3", "[.Meneldor_engine]")
{
  // London main line
  std::string fen = "r1bq1rk1/p4ppp/1pnbpn2/2ppN3/3P4/2PBP1B1/PP1N1PPP/R2QK2R b KQ - 1 9";
  engine_stats_from_position(fen);
}

TEST_CASE("Search_end1", "[.Meneldor_engine]")
{
  std::string fen = "5q2/n2P1k2/2b5/8/8/3N4/4BK2/6Q1 w - - 0 1";
  engine_stats_from_position(fen);
}

TEST_CASE("Search_end2", "[.Meneldor_engine]")
{
  std::string fen = "8/6p1/k1P2p1p/7K/8/8/8/8 w - - 0 1";
  engine_stats_from_position(fen);
}

TEST_CASE("Search_end3", "[.Meneldor_engine]")
{
  // Knight underpromotion
  std::string fen = "8/q1P1k3/8/8/8/8/5PP1/6K1 w - - 0 1";
  engine_stats_from_position(fen);
}

TEST_CASE("Search_mate1", "[.Meneldor_engine]")
{
  std::string fen = "k5r1/8/8/8/7K/5q2/7P/8 b - - 0 1";

  Meneldor_engine engine;
  engine.setDebug(false);
  engine.initialize();
  engine.setPosition(fen);

  senjo::GoParams params;
  params.depth = 5;

  auto best_move = engine.go(params);
  REQUIRE(best_move == "f3g4");
}

TEST_CASE("Search_repetition", "[.Meneldor_engine]")
{
  std::string fen = "4k3/p6q/8/7N/8/7P/PP3PP1/R5K1 w - - 0 1";

  Meneldor_engine engine;
  engine.setDebug(false);
  engine.initialize();
  engine.setPosition(fen);

  senjo::GoParams params;
  params.depth = 5;
  auto best_move = engine.go(params);

  // The best move is f6 to fork the king and queen
  REQUIRE(best_move == "h5f6");

  engine.makeMove("h5f6");
  engine.makeMove("e8f8");
  engine.makeMove("f6h5");
  engine.makeMove("f8e8");

  best_move = engine.go(params);

  // Now f6 would be a draw by repetition, so the engine should find something else
  REQUIRE(best_move != "h5f6");
}
