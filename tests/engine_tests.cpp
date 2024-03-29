#include <catch2/catch.hpp>

#include "meneldor_engine.h"
#include "senjo/UCIAdapter.h"
#include "utils.h"

namespace rs = std::ranges;
namespace Meneldor
{
auto engine_stats_from_position(std::string_view fen, int depth = 9, bool debug = false)
{
  static std::string const c_performance_log_filename{"output/performance_log.txt"};
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
  params.depth = depth;
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

TEST_CASE("Evaluate", "[Meneldor_engine]")
{
  std::string fen = "r1bqk2r/p2p1pbp/1pn3p1/1p1Np2n/4PP2/P2P4/1PP1N1PP/R1B2RK1 b kq f3 0 10";
  auto board = *Board::from_fen(fen);

  Meneldor_engine engine;

  // The evaluation function will change over time, but black is clearly winning
  // in this position
  // Black to move -> should return a positive value to indicate black is better
  REQUIRE(engine.evaluate(board) > 0);
}

TEST_CASE("Search_opening", "[.Meneldor_engine]")
{
  engine_stats_from_position(c_start_position_fen);
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

TEST_CASE("Best_of_several_mates", "[.Meneldor_engine]")
{
  std::string fen{"3k4/8/n7/6p1/1p2bq2/7r/8/4K3 b - - 0 1"};
  Meneldor_engine engine;
  engine.initialize();
  engine.setPosition(fen);

  senjo::GoParams params;
  params.depth = 8;

  auto best_move = engine.go(params);

  // Three possible mate in two moves
  bool result = (best_move == "f4e3" || best_move == "h3h2" || best_move == "e4d3");
  REQUIRE(result);
}

TEST_CASE("Mate_in_3_attack", "[.Meneldor_engine]")
{
  std::string fen{"r5n1/ppp1q3/2bp2kp/5rP1/3Qp3/2N5/PPP1B3/2KR3R w - - 0 1"};
  Meneldor_engine engine;
  engine.initialize();
  engine.setPosition(fen);

  senjo::GoParams params;
  params.depth = 6;

  auto best_move = engine.go(params);

  std::vector<std::string> const expected_pv{"e2h5", "g6g5", "d1g1", "g5f4", "c3e2"};
  auto actual_pv = engine.get_principal_variation("e2h5");

  bool match = actual_pv.has_value() && rs::equal(expected_pv, *actual_pv);
  REQUIRE(match);
}

TEST_CASE("Mate_in_2_defend", "[.Meneldor_engine]")
{
  std::string fen{"r5n1/ppp1q3/2bp2kp/5rPB/3Qp3/2N5/PPP5/2KR3R b - - 1 1"};
  Meneldor_engine engine;
  engine.initialize();
  engine.setPosition(fen);

  senjo::GoParams params;
  params.depth = 5;

  auto best_move = engine.go(params);

  std::vector<std::string> const expected_pv{"g6g5", "d1g1", "g5f4", "c3e2"};
  auto actual_pv = engine.get_principal_variation("g6g5");

  bool match = actual_pv.has_value() && rs::equal(expected_pv, *actual_pv);
  REQUIRE(match);
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

TEST_CASE("Search_ponder_move", "[.Meneldor_engine]")
{
  std::string fen = "8/q1P1k3/8/8/8/8/5PP1/6K1 w - - 0 1";

  Meneldor_engine engine;
  engine.setDebug(false);
  engine.initialize();
  engine.setPosition(fen);

  senjo::GoParams params;
  params.depth = 5;

  std::string ponder;
  auto best_move = engine.go(params, &ponder);

  engine.makeMove(best_move);
  params.depth--;
  best_move = engine.go(params, nullptr);

  REQUIRE(ponder == best_move);
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

  // The best move is Nf6 to fork the king and queen
  REQUIRE(best_move == "h5f6");

  engine.makeMove("h5f6");
  engine.makeMove("e8f8");
  engine.makeMove("f6h5");
  engine.makeMove("f8e8");

  best_move = engine.go(params);

  // Now f6 would be a draw by repetition, so the engine should find something
  // else
  REQUIRE(best_move != "h5f6");
}

TEST_CASE("Search_repetition_uci", "[.Meneldor_engine]")
{
  Meneldor_engine engine;
  senjo::UCIAdapter adapter{engine};

  std::vector<std::string> commands{
    "position fen 4Q3/p3B1pk/1p2p2p/2p4P/P1b1pP2/4n1K1/3r2P1/6NR w - - 0 31", "go depth 8",
    "position fen 8/p3B1pk/1p2p1Qp/2p4P/P1b1pP2/4n1K1/3r2P1/6NR b - - 1 31",  "go depth 3",
    "position fen 7k/p3B1p1/1p2p1Qp/2p4P/P1b1pP2/4n1K1/3r2P1/6NR w - - 2 32", "go depth 8",
    "position fen 8/p3B1pk/1p2p1Qp/2p4P/P1b1pP2/4n1K1/3r2P1/6NR b - - 3 32",  "go depth 3",
    "position fen 4Q3/p3B1pk/1p2p2p/2p4P/P1b1pP2/4n1K1/3r2P1/6NR w - - 4 33",
  };

  for (auto const& cmd : commands)
  {
    adapter.doCommand(cmd);
  }

  senjo::GoParams params;
  params.depth = 8;
  auto best_move = engine.go(params);
  REQUIRE(best_move != "e8g6");
}

TEST_CASE("No pseudo legal moves", "[.Meneldor_engine]")
{
  Meneldor_engine engine;
  engine.initialize();
  SECTION("No moves")
  {
    std::string fen = "6k1/8/8/6p1/5pPp/5PRP/5PRQ/6BK w - - 0 1";
    engine.setPosition(fen);

    senjo::GoParams params;
    auto m = engine.go(params);
    params.depth = 6;

    REQUIRE(m == "");
  }

  SECTION("Black can force a position with no moves")
  {
    std::string fen = "6k1/8/6p1/8/5pPp/5PRP/5PRQ/6BK b - - 0 1";
    engine.setPosition(fen);

    senjo::GoParams params;
    auto m = engine.go(params);
    params.depth = 6;

    REQUIRE(m == "g6g5");
  }

  SECTION("Two moves away from position with no moves")
  {
    std::string fen = "6k1/8/6p1/8/5pPp/5nRP/4PPRQ/6BK w - - 0 1";
    engine.setPosition(fen);

    senjo::GoParams params;
    auto m = engine.go(params);
    params.depth = 6;

    // No requires, only make sure we don't infinite loop or crash
  }
}
} // namespace Meneldor
