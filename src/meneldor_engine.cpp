#include "meneldor_engine.h"
#include "feature_toggle.h"
#include "move_generator.h"
#include "senjo/Output.h"
#include "utils.h"

namespace
{
constexpr int positive_inf = std::numeric_limits<int>::max();
constexpr int negative_inf = -positive_inf;
static_assert(positive_inf == -negative_inf, "Values should be inverses of each other");

int tt_hits{0};
int tt_misses{0};
int tt_sufficient_depth{0};

} // namespace

// Returns a number that is positive if the side to move is winning, and negative if losing
int Meneldor_engine::evaluate(Board const& board) const
{
  // These arrays can be iterated in parallel
  constexpr static std::array piece_values{100, 300, 300, 500, 900};
  constexpr static std::array pieces{Piece::pawn, Piece::knight, Piece::bishop, Piece::rook, Piece::queen};
  static_assert(piece_values.size() == pieces.size());

  auto const state = board.calc_game_state();
  if (state == Game_state::white_victory || state == Game_state::black_victory)
  {
    // Add depth so the search function can return a slightly higher value if it finds an earlier mate
    return negative_inf + m_depth_for_current_search;
  }
  if (state == Game_state::draw || board.get_halfmove_clock() >= 100)
  {
    return c_contempt_score;
  }

  auto const color = board.get_active_color();
  auto const enemy_color = opposite_color(color);
  int result{0};
  for (size_t i{0}; i < pieces.size(); ++i)
  {
    result +=
      (board.get_piece_set(color, pieces[i]).occupancy() - board.get_piece_set(enemy_color, pieces[i]).occupancy()) *
      piece_values[i];
  }

  // Positions that can attack more squares are better
  result += Move_generator::get_all_attacked_squares(board, board.get_active_color()).occupancy();

  return result;
}

int Meneldor_engine::quiesce_(Board const& board, int alpha, int beta) const
{
  ++m_visited_quiesence_nodes;
  auto score = evaluate(board);
  if (score >= beta)
  {
    return beta;
  }
  alpha = std::max(alpha, score);

  auto moves = Move_generator::generate_pseudo_legal_attack_moves(board);
  m_orderer.sort_moves(moves, board);

  Board tmp_board{board};
  for (auto const move : moves)
  {
    tmp_board = board;
    tmp_board.move_no_verify(move);
    if (tmp_board.is_in_check(opposite_color(tmp_board.get_active_color())))
    {
      continue;
    }
    score = -quiesce_(tmp_board, -beta, -alpha);

    if (score >= beta)
    {
      return beta;
    }
    alpha = std::max(score, alpha);
  }

  return alpha;
}

bool Meneldor_engine::has_more_time_() const
{
  if (m_search_mode != Search_mode::time)
  {
    return true;
  }

  auto const current_time = std::chrono::system_clock::now();
  std::chrono::duration<double> const elapsed_time = current_time - m_search_start_time;
  return elapsed_time < m_time_for_move;
}

void Meneldor_engine::calc_time_for_move_(senjo::GoParams const& params)
{
  // We won't exit right away once time has expired, so include a buffer 
  // so we can return a move in time
  constexpr double c_percent_time_to_use{0.95};

  if (params.movetime > 0)
  {
    m_time_for_move = std::chrono::milliseconds{params.movetime} * c_percent_time_to_use;
    return;
  }

  int our_time = params.wtime;
  int their_time = params.btime;
  int our_increment = params.winc;
  int their_increment = params.winc;
  if (m_board.get_active_color() == Color::black)
  {
    std::swap(our_time, their_time);
    std::swap(our_increment, their_increment);
  }

  int moves_to_go = params.movestogo;
  if (moves_to_go == 0)
  {
    constexpr int c_estimated_moves_to_go{20};

    // We need to move faster if our opponent has more time than we do
    double const time_ratio = std::max((static_cast<double>(our_time) / their_time), 1.0);
    moves_to_go = static_cast<int>(c_estimated_moves_to_go * std::min(2.0, time_ratio));
  }
  
  // Slight fudge factor to ensure that we don't go over when moves_to_go is small
  moves_to_go += 3;

  m_time_for_move = std::chrono::milliseconds{((m_board.get_active_color() == Color::black) ? params.btime : params.wtime) / (moves_to_go)};
  m_time_for_move += std::chrono::milliseconds{our_increment};
  m_time_for_move *= c_percent_time_to_use;
}

int Meneldor_engine::negamax_(Board& board, int alpha, int beta, int depth_remaining)
{
  ++m_visited_nodes;

  if (depth_remaining == 0)
  {
    return quiesce_(board, alpha, beta);
  }

  if (std::find(m_previous_positions.cbegin(), m_previous_positions.cend(), board.get_hash_key()) !=
      m_previous_positions.cend())
  {
    return c_contempt_score; // Draw by repetition
  }

  Move best_guess{};
  if (auto entry = m_transpositions.get(board.get_hash_key()))
  {
    ++tt_hits;
    if (entry->depth >= depth_remaining)
    {
      ++tt_sufficient_depth;
      switch (entry->type)
      {
        case Transposition_table::Eval_type::alpha:
          // Eval_type::alpha implies we didn't find a move from this position that was as good as a move that
          // we could have made earlier to lead to a different position. That means the position has an evaluation
          // that is at most "entry.evaluation"
          beta = std::min(beta, entry->evaluation);
          break;
        case Transposition_table::Eval_type::beta:
          // Eval_type::beta implies that we stopped evaluating last time because we didn't think the opposing player
          // would allow this position to be reached. That means the position has an evaluation of at least "entry.evaluation",
          // but there may be an even better move that was skipped
          alpha = std::max(alpha, entry->evaluation);
          break;
        case Transposition_table::Eval_type::exact:
          --m_visited_nodes; //TODO: Is this useful?
          return entry->evaluation;
          break;
      }
    }
    else if (entry->best_move.type() != Move_type::null)
    {
      // TODO: Set the best move as the first one in our search; even though we need to search to a
      // larger depth it still has a good chance of being the best move
      best_guess = entry->best_move;
      //std::cout << "Move: " << best_guess << "\n";
    }
  }
  else
  {
    ++tt_misses;
  }

  auto moves = Move_generator::generate_pseudo_legal_moves(board);
  m_orderer.sort_moves(moves, board);

  //TODO: Test this
  static const bool skip_guess_move = is_feature_enabled("skip_guess_move");
  if (!skip_guess_move)
  {
    if (best_guess.type() != Move_type::null)
    {
      auto const guess_location = std::find(moves.begin(), moves.end(), best_guess);
      if (guess_location != moves.end())
      {
        //std::cout << "Move list (size: " << moves.size() << "): ";
        //print_vector(std::cout, moves);
        //MY_ASSERT(guess_location != moves.end(), "Move should always be present");
        std::rotate(moves.begin(), guess_location, guess_location + 1);
      }
      else
      {
        // Zobrist key collision?
      }
    }
  }

  // If we don't find a move here that's better than alpha, just save alpha as the upper bound for this position
  bool has_any_moves{false};
  auto eval_type = Transposition_table::Eval_type::alpha;
  Move best{moves.front()};
  Board tmp_board{board};
  for (auto const move : moves)
  {
    tmp_board = board;
    tmp_board.move_no_verify(move);
    if (tmp_board.is_in_check(opposite_color(tmp_board.get_active_color())))
    {
      continue;
    }
    has_any_moves = true;

    auto const score = -negamax_(tmp_board, -beta, -alpha, depth_remaining - 1);

    if (score >= beta)
    {
      // Stop evaluating here since the opposing player won't let us get even this position on their previous move
      // Our evaluation here is a lower bound
      eval_type = Transposition_table::Eval_type::beta;
      m_transpositions.insert(board.get_hash_key(), {board.get_hash_key(), depth_remaining, score, move, eval_type});

      return beta;
    }

    if (score > alpha)
    {
      alpha = score;
      best = move;

      // If this is never hit, we know that the best alpha can be is the alpha that was passed into the function
      eval_type = Transposition_table::Eval_type::exact;
    }

    if (!has_more_time_())
    {
      break;
    }
  }

  if (!has_any_moves)
  {
    if (board.is_in_check(board.get_active_color()))
    {
      return negative_inf + (m_depth_for_current_search - depth_remaining);
    }
    return c_contempt_score;
  }

  m_transpositions.insert(board.get_hash_key(), {board.get_hash_key(), depth_remaining, alpha, best, eval_type});
  return alpha;
}

Meneldor_engine::Meneldor_engine()
{
  m_previous_positions.reserve(256);
}

std::string Meneldor_engine::getEngineName() const
{
  return "Meneldor";
}

std::string Meneldor_engine::getEngineVersion() const
{
  return "0.1";
}

std::string Meneldor_engine::getAuthorName() const
{
  return "Jeremy Sigrist";
}

std::string Meneldor_engine::getEmailAddress() const
{
  return "";
}

std::string Meneldor_engine::getCountryName() const
{
  return "USA";
}

std::list<senjo::EngineOption> Meneldor_engine::getOptions() const
{
  // This engine does not currently advertise any options / settings
  return {};
}

bool Meneldor_engine::setEngineOption(const std::string& /* optionName */, const std::string& /* optionValue */)
{
  return false;
}

void Meneldor_engine::initialize()
{
  m_board = {};
}

bool Meneldor_engine::isInitialized() const
{
  return true;
}

bool Meneldor_engine::setPosition(const std::string& fen, std::string* /* remain  = nullptr */)
{
  //TODO: Use Output() to report errors in fen string
  if (auto board = Board::from_fen(fen))
  {
    m_board = *board;
    m_previous_positions.clear();
    return true;
  }

  return false;
}

bool Meneldor_engine::makeMove(const std::string& move)
{
  if (m_board.try_move_uci(move))
  {
    if (m_board.get_halfmove_clock() == 0)
    {
      // If the halfmove clock was zero we just had a capture or pawn move, both
      // of which make repeating positions impossible. That means we can't repeat
      // any of the positions in the list, so clear it so we don't have to compare
      // against them going forward
      m_previous_positions.clear();
    }

    m_previous_positions.push_back(m_board.get_hash_key());
    return true;
  }

  return false;
}

std::string Meneldor_engine::getFEN() const
{
  return m_board.to_fen();
}

void Meneldor_engine::printBoard() const
{
  std::cout << m_board;
}

bool Meneldor_engine::whiteToMove() const
{
  return m_board.get_active_color() == Color::white;
}

void Meneldor_engine::clearSearchData()
{
  // No data persists between searches currently
}

void Meneldor_engine::ponderHit()
{
  // Notifies the engine that the ponder move was played
}

bool Meneldor_engine::isRegistered() const
{
  // This engine does not need to be registered to function
  return true;
}

void Meneldor_engine::registerLater()
{
  // This engine does not need to be registered to function
}

bool Meneldor_engine::doRegistration(const std::string& /* name */, const std::string& /* code */)
{
  // This engine does not need to be registered to function
  return true;
}

bool Meneldor_engine::isCopyProtected() const
{
  return false;
}

bool Meneldor_engine::copyIsOK()
{
  return true;
}

void Meneldor_engine::setDebug(const bool flag)
{
  m_is_debug = flag;
}

bool Meneldor_engine::isDebugOn() const
{
  return m_is_debug;
}

bool Meneldor_engine::isSearching()
{
  return m_is_searching.test();
}

void Meneldor_engine::stopSearching()
{
  m_stop_requested.test_and_set();
}

bool Meneldor_engine::stopRequested() const
{
  return m_stop_requested.test();
}

void Meneldor_engine::waitForSearchFinish()
{
  //constexpr bool old_value{true};
  //m_is_searching.wait(old_value);

  while (m_is_searching.test())
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
  }
}

uint64_t Meneldor_engine::perft(const int depth)
{
  m_stop_requested.clear();

  auto const start = std::chrono::system_clock::now();
  std::atomic_flag is_cancelled{false};
  auto const result = Move_generator::perft(depth, m_board, is_cancelled);
  auto const end = std::chrono::system_clock::now();
  std::chrono::duration<double> const elapsed = end - start;
  auto const elapsed_seconds = elapsed.count();

  std::cout << "perft(" << std::to_string(depth) << ") = " << std::to_string(result) << "\n"
            << "Elapsed time: " << std::to_string(elapsed_seconds) << " seconds\n"
            << "Nodes/sec: " << format_with_commas(result / elapsed_seconds) << "\n";

  return result;
}

std::pair<Move, int> Meneldor_engine::search(int depth, std::vector<Move>& legal_moves)
{
  MY_ASSERT(!legal_moves.empty(), "Already in checkmate or stalemate");
  constexpr static int c_depth_to_use_id_score{3};

  // TODO: Is this useful?
  // Currently use_id_sort appears to slightly slow down the engine, and shows no benefit over the MVV/LVA tables
  static const bool use_id_sort = is_feature_enabled("use_id_sort");
  if (use_id_sort && depth < c_depth_to_use_id_score)
  {
    m_orderer.sort_moves(legal_moves, m_board);
  }
  else
  {
    std::stable_sort(legal_moves.begin(), legal_moves.end(),
                     [](auto m1, auto m2)
                     {
                       return m1.score() > m2.score();
                     });
  }

  std::pair<Move, int> best{legal_moves.front(), negative_inf};
  for (auto& move : legal_moves)
  {
    auto tmp_board = m_board;
    tmp_board.move_no_verify(move);

    auto const score = -negamax_(tmp_board, negative_inf, positive_inf, depth - 1);
    uint8_t score_four_bits = std::clamp((score / 200) + 7, 0, 15);
    move.set_score(score_four_bits);

    if (m_is_debug)
    {
      std::cout << "Evaluating move: " << move << ", score: " << std::to_string(score) << "\n";
    }

    if (score > best.second)
    {
      best = {move, score};
    }

    if (m_stop_requested.test())
    {
      break;
    }
  }

  return best;
}

void Meneldor_engine::print_stats(std::pair<Move, int> best_move) const
{
  /*
  Example output from stockfish:
  info depth 1 seldepth 1 multipv 1 score cp -18 nodes 22 nps 7333 tbhits 0 time 3 pv e7e5
  info depth 2 seldepth 2 multipv 1 score cp 3 nodes 43 nps 14333 tbhits 0 time 3 pv e7e5 a2a3
  */

  auto const stats = getSearchStats();
  auto const nodes_per_second =
    (stats.msecs == 0) ? 0 : static_cast<int32_t>(1000.0 * static_cast<double>(stats.nodes) / stats.msecs);
  std::stringstream out;
  out << "info depth " << stats.depth << " seldepth " << stats.seldepth << " score cp " << best_move.second << " nodes "
      << stats.nodes << " nps " << nodes_per_second << " time " << stats.msecs;

  if (auto pv = get_principle_variation(move_to_string(best_move.first)))
  {
    out << " pv ";
    for (auto const& m : *pv)
    {
      out << m << " ";
    }
  }

  // Output() adds the prefix "info string" by default to make UCI clients ignore the info. We want to
  // use the "info" prefix so UCI clients can parse the search info, so we add that prefix manually.
  senjo::Output(senjo::Output::OutputPrefix::NoPrefix) << out.str();
}

std::string Meneldor_engine::go(const senjo::GoParams& params, std::string* /* ponder = nullptr */)
{
  m_search_start_time = std::chrono::system_clock::now();

  m_visited_nodes = 0;
  m_visited_quiesence_nodes = 0;
  m_stop_requested.clear();
  m_is_searching.test_and_set();

  if (m_is_debug)
  {
    auto const color = m_board.get_active_color();
    std::cout << "Eval of current position (for " << color << "): " << std::to_string(evaluate(m_board)) << "\n";
  }

  calc_time_for_move_(params);
  auto legal_moves = Move_generator::generate_legal_moves(m_board);

  int const max_depth = (params.depth > 0) ? params.depth : c_default_depth;
  m_search_mode = Search_mode::depth;
  if (params.wtime > 0 || params.btime > 0)
  {
    m_search_mode = Search_mode::time;
  }

  std::pair<Move, int> best_move;
  for (int depth{std::min(2, max_depth)}; depth <= max_depth; ++depth)
  {
    m_depth_for_current_search = depth;

    best_move = search(m_depth_for_current_search, legal_moves);
    print_stats(best_move);
    if (!has_more_time_())
    {
      if (m_is_debug)
      {
        std::cout << "Searched to depth: " << depth << "\n";
      }
      break;
    }
  }

  m_search_end_time = std::chrono::system_clock::now();

  if (m_is_debug)
  {
    std::cout << "Search depth: " << m_depth_for_current_search << "\n";
    std::cout << "TT occupancy: " << m_transpositions.count() << "\n";
    std::cout << "TT percent full: " << (static_cast<float>(m_transpositions.count()) / m_transpositions.get_capacity())
              << "\n";

    std::cout << "TT Hits: " << tt_hits << ", total: " << (tt_hits + tt_misses)
              << ", sufficient_depth: " << tt_sufficient_depth
              << ", hit%: " << (static_cast<float>(tt_hits) / (tt_hits + tt_misses)) << "\n";

    tt_hits = 0;
    tt_misses = 0;
    tt_sufficient_depth = 0;
  }

  m_is_searching.clear();
  return move_to_string(best_move.first);
}

senjo::SearchStats Meneldor_engine::getSearchStats() const
{
  senjo::SearchStats result;

  result.depth = m_depth_for_current_search;
  result.seldepth = m_depth_for_current_search;
  result.nodes = m_visited_nodes;
  result.qnodes = m_visited_quiesence_nodes;

  auto const end_time = m_is_searching.test() ? std::chrono::system_clock::now() : m_search_end_time;
  std::chrono::duration<double> const elapsed = (end_time - m_search_start_time);
  result.msecs = static_cast<uint64_t>(elapsed.count() * 1000.0);

  return result;
}

void Meneldor_engine::resetEngineStats()
{
  // Called when "test" command is received
}

void Meneldor_engine::showEngineStats() const
{
  // Called when "test" command is received
}

// Should be called after go() but before makeMove()
std::optional<std::vector<std::string>> Meneldor_engine::get_principle_variation(std::string move_str) const
{
  std::vector<std::string> result;
  Board tmp_board{m_board};
  auto next_move = tmp_board.move_from_uci(std::move(move_str));

  auto depth = m_depth_for_current_search;
  while (depth > 1)
  {
    if (!tmp_board.try_move(*next_move))
    {
      return {};
    }

    auto entry = m_transpositions.get(tmp_board.get_hash_key());
    if (!entry)
    {
      return {};
    }
    result.push_back(move_to_string(*next_move));
    depth = entry->depth;
    next_move = entry->best_move;

    if (entry->type != Transposition_table::Eval_type::exact)
    {
      return {};
    }
  }

  return result;
}
