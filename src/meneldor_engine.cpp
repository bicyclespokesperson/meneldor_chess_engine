#include "meneldor_engine.h"
#include "feature_toggle.h"
#include "move_generator.h"
#include "senjo/Output.h"
#include "utils.h"

namespace rs = std::ranges;

namespace
{
int tt_hits{0};
int tt_misses{0};
int tt_sufficient_depth{0};

} // namespace

// Returns a number that is positive if the side to move is winning, and
// negative if losing
int Meneldor_engine::evaluate(Board const& board) const
{
  // These arrays can be iterated in parallel
  constexpr static std::array piece_values{100, 300, 300, 500, 900};
  constexpr static std::array pieces{Piece::pawn, Piece::knight, Piece::bishop, Piece::rook, Piece::queen};
  static_assert(piece_values.size() == pieces.size());

  auto const state = board.calc_game_state();
  if (state == Game_state::white_victory || state == Game_state::black_victory)
  {
    // Add depth so the search function can return a slightly higher value if it
    // finds an earlier mate
    return negative_inf + m_depth_for_current_search;
  }
  if (state == Game_state::draw || board.get_halfmove_clock() >= 100)
  {
    return c_contempt_score;
  }

  auto const color = board.get_active_color();
  auto const enemy_color = opposite_color(color);
  int material_result{0};
  for (size_t i{0}; i < pieces.size(); ++i)
  {
    material_result +=
      (board.get_piece_set(color, pieces[i]).occupancy() - board.get_piece_set(enemy_color, pieces[i]).occupancy()) *
      piece_values[i];
  }

  // Positions that can attack more squares are better
  auto const mobility_result = Move_generator::get_all_attacked_squares(board, board.get_active_color()).occupancy();
  auto const result = material_result + mobility_result;
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
  if (stopRequested())
  {
    return false;
  }
  return (m_search_mode != Search_mode::time) || std::chrono::system_clock::now() < m_search_desired_end_time;
}

void Meneldor_engine::calc_time_for_move_(senjo::GoParams const& params)
{
  // We won't exit right away once time has expired, so include a buffer
  // so we can return a move in time
  constexpr double c_percent_time_to_use{0.95};

  std::chrono::milliseconds time_for_move{0};
  if (params.infinite)
  {
    time_for_move = std::chrono::years{1};
  }
  else if (params.movetime > 0)
  {
    time_for_move = std::chrono::milliseconds{static_cast<int>(c_percent_time_to_use * params.movetime)};
  }
  else
  {
    auto our_time = params.wtime;
    auto their_time = params.btime;
    auto our_increment = params.winc;
    auto their_increment = params.winc;
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
      double const time_ratio = std::clamp((static_cast<double>(their_time) / our_time), 1.0, 2.0);
      moves_to_go = static_cast<int>(c_estimated_moves_to_go * time_ratio);
    }

    time_for_move = std::chrono::milliseconds{static_cast<int>(c_percent_time_to_use * our_time / moves_to_go)};
    time_for_move += std::chrono::milliseconds{our_increment};
  }

  m_search_desired_end_time = m_search_start_time + time_for_move;
}

int Meneldor_engine::negamax_(Board& board,
                              int alpha,
                              int beta,
                              int depth_remaining,
                              bool previous_move_was_null /* = false */)
{
  ++m_visited_nodes;
  if (!has_more_time_())
  {
    m_search_timed_out = true;
    return 0;
  }

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
  if (auto const entry = m_transpositions.get(board.get_hash_key()))
  {
    best_guess = entry->best_move;
    ++tt_hits;

    if (entry->depth >= depth_remaining)
    {
      ++tt_sufficient_depth;
      switch (entry->type)
      {
        case Transposition_table::Eval_type::alpha:
          // Eval_type::alpha implies we didn't find a move from this position
          // that lead to a better state than a state we could have reached with a different earlier move.
          // That means the position has an evaluation that is at most "entry.evaluation"
          beta = std::min(beta, entry->evaluation);
          break;
        case Transposition_table::Eval_type::beta:
          // Eval_type::beta implies that we stopped evaluating last time because
          // we didn't think the opposing player would allow this position to be
          // reached. That means the position has an evaluation of at least
          // "entry.evaluation", but there may be an even better move that was skipped
          alpha = std::max(alpha, entry->evaluation);
          break;
        case Transposition_table::Eval_type::exact:
          return entry->evaluation;
      }
    }
    else if (entry->best_move.type() != Move_type::null)
    {
      best_guess = entry->best_move;
    }
  }
  else
  {
    ++tt_misses;
  }

  // Don't use null move pruning if the node is part of the principal variation
  bool const is_pv_node = (beta - alpha != 1);

  // Null move pruning
  constexpr int c_min_depth_for_null_move_pruning{4};
  static bool const skip_null_move_pruning = is_feature_enabled("skip_null_move_pruning");

  if (depth_remaining >= c_min_depth_for_null_move_pruning && !skip_null_move_pruning && !is_pv_node &&
      !previous_move_was_null && !board.is_in_check(board.get_active_color()))
  {
    if (auto eval = evaluate(board); eval > beta)
    {
      int const r = 2;

      Move null_move{};
      Board tmp_board{board};
      tmp_board.move_no_verify(null_move, true);

      constexpr bool previous_was_null{true};
      int const null_score = -negamax_(tmp_board, -beta, -alpha, depth_remaining - 1 - r, previous_was_null);
      if (null_score >= beta)
      {
        //TODO: Perform full search to verify?
        return beta;
      }
    }
  }

  auto moves = Move_generator::generate_pseudo_legal_moves(board);
  m_orderer.sort_moves(moves, board);

  static const bool skip_guess_move = is_feature_enabled("skip_guess_move");
  if (!skip_guess_move)
  {
    if (best_guess.type() != Move_type::null)
    {
      auto const guess_location = std::find(moves.begin(), moves.end(), best_guess);
      if (guess_location != moves.end())
      {
        std::rotate(moves.begin(), guess_location, guess_location + 1);
      }
      else
      {
        // Zobrist key collision
      }
    }
  }

  // If we don't find a move here that's better than alpha, just save alpha as
  // the upper bound for this position
  bool has_any_moves{false};
  bool perform_full_search{true};
  auto eval_type = Transposition_table::Eval_type::alpha;

  // This list can't be empty, wouldn't have gotten this far if it was
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

    int score{0};
    if (perform_full_search)
    {
      score = -negamax_(tmp_board, -beta, -alpha, depth_remaining - 1);
    }
    else
    {
      score = -negamax_(tmp_board, -alpha - 1, -alpha, depth_remaining - 1);
      if (alpha < score && score < beta)
      {
        // If we found a better move than our previous best move, perform a full search to get its accurate value
        score = -negamax_(tmp_board, -beta, -score, depth_remaining - 1); // -score instead of -alpha?
      }
    }

    perform_full_search = false;

    if (score >= beta)
    {
      // Stop evaluating here since the opposing player won't let us get even this position on their previous move.
      // Our evaluation here is a lower bound

      eval_type = Transposition_table::Eval_type::beta;
      m_transpositions.insert(board.get_hash_key(), {board.get_hash_key(), depth_remaining, score, move, eval_type});

      return beta;
    }

    if (score > alpha)
    {
      alpha = score;
      best = move;

      // If this is never hit, we know that the best alpha can be is the alpha
      // that was passed into the function
      eval_type = Transposition_table::Eval_type::exact;
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

bool Meneldor_engine::setEngineOption(std::string const& /* optionName */, std::string const& /* optionValue */)
{
  return false;
}

void Meneldor_engine::initialize()
{
  m_board = {};
  m_previous_positions.clear();
}

bool Meneldor_engine::isInitialized() const
{
  return true;
}

bool Meneldor_engine::setPosition(std::string const& fen, std::string* remain /* = nullptr */)
{
  auto board = Board::from_fen(fen);
  if (board)
  {
    m_board = std::move(*board);
    m_previous_positions.push_back(m_board.get_hash_key());

    if (remain)
    {
      auto const move_index = fen.find("moves");
      if (move_index < std::string::npos)
      {
        *remain = fen.substr(move_index);
      }
    }

    return true;
  }

  senjo::Output() << board.error() << "\n";
  return false;
}

bool Meneldor_engine::makeMove(std::string const& move)
{
  if (m_board.try_move_uci(move))
  {
    if (m_board.get_halfmove_clock() == 0)
    {
      // If the halfmove clock was zero we just had a capture or pawn move, both
      // of which make repeating positions impossible. That means we can't
      // repeat any of the positions in the list, so clear it so we don't have
      // to compare against them going forward
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
  senjo::Output(senjo::Output::OutputPrefix::NoPrefix) << m_board;
}

bool Meneldor_engine::whiteToMove() const
{
  return m_board.get_active_color() == Color::white;
}

void Meneldor_engine::clearSearchData()
{
  // No data persists between searches currently
  m_transpositions.clear();
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

bool Meneldor_engine::doRegistration(std::string const& /* name */, std::string const& /* code */)
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

void Meneldor_engine::setDebug(bool const flag)
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
  while (m_is_searching.test())
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
  }
}

uint64_t Meneldor_engine::perft(int const depth)
{
  m_stop_requested.clear();

  auto const start = std::chrono::system_clock::now();
  std::atomic_flag is_cancelled{};
  auto const result = Move_generator::perft(depth, m_board, is_cancelled);
  auto const end = std::chrono::system_clock::now();
  std::chrono::duration<double> const elapsed = end - start;
  auto const elapsed_seconds = elapsed.count();

  senjo::Output(senjo::Output::OutputPrefix::NoPrefix)
    << "perft(" << std::to_string(depth) << ") = " << std::to_string(result) << "\n"
    << "Elapsed time: " << std::to_string(elapsed_seconds) << " seconds\n"
    << "Nodes/sec: " << format_with_commas(result / elapsed_seconds) << "\n";

  return result;
}

std::pair<Move, int> Meneldor_engine::search(int depth, std::vector<Move>& legal_moves)
{
  MY_ASSERT(!legal_moves.empty(), "Already in checkmate or stalemate");
  constexpr static int c_depth_to_use_id_score{3};

  // TODO: Is this useful? Revisit after null move pruning is implemented
  // Currently use_id_sort appears to slightly slow down the engine, and shows
  // no benefit over the MVV/LVA tables
  static const bool skip_id_sort = is_feature_enabled("skip_id_sort");
  if (skip_id_sort || depth < c_depth_to_use_id_score)
  {
    m_orderer.sort_moves(legal_moves, m_board);
  }
  else
  {
    std::stable_sort(legal_moves.begin(), legal_moves.end(),
                     [](Move m1, Move m2)
                     {
                       return m1.score() > m2.score();
                     });
  }

  std::string move_string{""};
  bool perform_full_search{true};
  std::pair<Move, int> best{legal_moves.front(), negative_inf};
  for (auto& move : legal_moves)
  {
    auto tmp_board = m_board;
    tmp_board.move_no_verify(move);
    move_string = move_to_string(move);

    int score{0};
    if (perform_full_search)
    {
      score = -negamax_(tmp_board, negative_inf, positive_inf, depth - 1);
    }
    else
    {
      score = -negamax_(tmp_board, -best.second - 1, -best.second, depth - 1);
      if (score > best.second)
      {
        score = -negamax_(tmp_board, negative_inf, -score, depth - 1);
      }
    }
    perform_full_search = false;

    auto score_four_bits = static_cast<uint8_t>(std::clamp((score / 200) + 7, 0, 15));
    move.set_score(score_four_bits);

    if (m_is_debug)
    {
      std::cout << "Evaluating move: " << move << ", score: " << std::to_string(score) << "\n";
    }

    if (score > best.second && move_string.size() > 0)
    {
      best = {move, score};
    }
  }

  return best;
}

void Meneldor_engine::print_stats(std::pair<Move, int> best_move, std::optional<std::vector<std::string>> const& pv)
{
  /*
     Example output from stockfish:
     info depth 1 seldepth 1 multipv 1 score cp -18 nodes 22 nps 7333 tbhits 0 time 3 pv e7e5 
     info depth 2 seldepth 2 multipv 1 score cp 3 nodes 43 nps 14333 tbhits 0 time 3 pv e7e5 a2a3
     */

  auto format_score = [](int score)
  {
    if (score > c_max_non_mate_score)
    {
      return std::string{"mate "} + std::to_string(static_cast<int>(std::ceil((positive_inf - score) / 2.0)));
    }
    if (score < c_min_non_mate_score)
    {
      return std::string{"mate "} + std::to_string(static_cast<int>(std::ceil((negative_inf - score) / 2.0)));
    }
    return std::string{"cp "} + std::to_string(score);
  };

  auto const stats = getSearchStats();
  auto const nodes_per_second =
    (stats.msecs == 0) ? 0 : static_cast<int32_t>(1000.0 * static_cast<double>(stats.nodes) / stats.msecs);
  std::stringstream out;
  out << "info depth " << stats.depth << " seldepth " << stats.seldepth << " score " << format_score(best_move.second)
      << " nodes " << stats.nodes << " nps " << nodes_per_second << " time " << stats.msecs;

  if (pv)
  {
    out << " pv ";
    std::copy(pv->cbegin(), pv->cend(), std::ostream_iterator<std::string>(out, " "));
  }

  // Output() adds the prefix "info string" by default to make UCI clients
  // ignore the info. We want to use the "info" prefix so UCI clients can parse
  // the search info, so we add that prefix manually.
  senjo::Output(senjo::Output::OutputPrefix::NoPrefix) << out.str();
}

std::string Meneldor_engine::go(senjo::GoParams const& params, std::string* ponder)
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

  auto legal_moves = Move_generator::generate_legal_moves(m_board);
  if (legal_moves.empty())
  {
    senjo::Output(senjo::Output::OutputPrefix::NoPrefix) << "info string no legal moves";
    return {};
  }

  int max_depth = (params.depth > 0) ? params.depth : c_default_depth;
  m_search_mode = Search_mode::depth;
  if (params.wtime > 0 || params.btime > 0 || params.movetime > 0 || params.infinite)
  {
    calc_time_for_move_(params);
    m_search_mode = Search_mode::time;
    max_depth = c_max_supported_depth;
  }

  // Iterative deepening loop
  std::pair<Move, int> best_move;
  for (int depth{std::min(2, max_depth)}; has_more_time_() && (depth <= max_depth); ++depth)
  {
    m_search_timed_out = false;
    m_depth_for_current_search = depth;

    auto const move_candidate = search(m_depth_for_current_search, legal_moves);
    MY_ASSERT(move_candidate.first.type() != Move_type::null, "Best move cannot be null");
    if (!m_search_timed_out)
    {
      best_move = move_candidate;
      m_current_pv = get_principal_variation(move_to_string(best_move.first));
      print_stats(best_move, m_current_pv);
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
              << ", hit%: " << (static_cast<float>(100.0 * tt_hits) / (tt_hits + tt_misses)) << "\n";

    tt_hits = 0;
    tt_misses = 0;
    tt_sufficient_depth = 0;
  }

  if (ponder && m_current_pv && m_current_pv->size() > 1)
  {
    MY_ASSERT(m_current_pv->front() == move_to_string(best_move.first), "Incorrect principle variation");
    *ponder = m_current_pv->at(1);
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
std::optional<std::vector<std::string>> Meneldor_engine::get_principal_variation(std::string move_str) const
{
  std::vector<std::string> result;
  result.push_back(move_str);
  bool found_mate{false};

  Board tmp_board{m_board};
  auto next_move = tmp_board.move_from_uci(std::move(move_str));

  auto depth = m_depth_for_current_search - 1;
  while (depth > 0)
  {
    if (!tmp_board.try_move(*next_move))
    {
      return {};
    }

    auto const entry = m_transpositions.get(tmp_board.get_hash_key());
    if (!entry && found_mate)
    {
      // Checkmate, no more moves to find
      return result;
    }
    if (!entry)
    {
      return result;
    }

    depth -= 1;
    next_move = entry->best_move;
    result.push_back(move_to_string(*next_move));
    if (std::abs(entry->evaluation) >= positive_inf - m_depth_for_current_search)
    {
      found_mate = true;
    }
  }

  return result;
}
