#ifndef MENELDOR_ENGINE
#define MENELDOR_ENGINE

#include "bitboard.h"
#include "board.h"
#include "chess_types.h"
#include "coordinates.h"
#include "move_orderer.h"
#include "senjo/ChessEngine.h"
#include "transposition_table.h"

namespace Meneldor
{
class Meneldor_engine : public senjo::ChessEngine
{
public:
  Meneldor_engine();

  std::string getEngineName() const override;

  std::string getEngineVersion() const override;

  std::string getAuthorName() const override;

  std::string getEmailAddress() const override;

  std::string getCountryName() const override;

  std::list<senjo::EngineOption> getOptions() const override;

  bool setEngineOption(std::string const& optionName, std::string const& optionValue) override;

  void initialize() override;

  bool isInitialized() const override;

  bool setPosition(std::string const& fen, std::string* remain = nullptr) override;

  bool makeMove(std::string const& move) override;

  std::string getFEN() const override;

  void printBoard() const override;

  bool whiteToMove() const override;

  void clearSearchData() override;

  void ponderHit() override;

  bool isRegistered() const override;

  void registerLater() override;

  bool doRegistration(std::string const& name, std::string const& code) override;

  bool isCopyProtected() const override;

  bool copyIsOK() override;

  void setDebug(bool const flag) override;

  bool isDebugOn() const override;

  bool isSearching() override;

  void stopSearching() override;

  bool stopRequested() const override;

  void waitForSearchFinish() override;

  uint64_t perft(int const depth) override;

  std::pair<Move, int> search(int depth, std::vector<Move>& legal_moves);

  std::string go(senjo::GoParams const& params, std::string* ponder = nullptr) override;

  senjo::SearchStats getSearchStats() const override;

  void resetEngineStats() override;

  void showEngineStats() const override;

  int evaluate(Board const& board) const;

  std::optional<std::vector<std::string>> get_principal_variation(std::string move_str) const;

  void print_stats(std::pair<Move, int> best_move, std::optional<std::vector<std::string>> const& pv);

private:
  enum class Search_mode
  {
    depth = 0,
    time,
  };

  int negamax_(Board& board, int alpha, int beta, int depth_remaining, bool previous_move_was_null = false);

  int quiesce_(Board const& board, int alpha, int beta) const;

  bool has_more_time_() const;
  void calc_time_for_move_(senjo::GoParams const& params);

  bool m_is_debug{false};
  bool m_search_timed_out{false};
  std::atomic_flag m_stop_requested{};
  std::atomic_flag m_is_searching{};
  Board m_board;
  Move_orderer m_orderer{};
  std::vector<zhash_t> m_previous_positions;

  constexpr static int c_default_depth{6};
  int m_depth_for_current_search{c_default_depth};

  constexpr static size_t c_transposition_table_size_bytes{128UL * 1024UL * 1024UL};
  Transposition_table m_transpositions{c_transposition_table_size_bytes};
  std::optional<std::vector<std::string>> m_current_pv;

  mutable uint32_t m_visited_nodes{0};
  mutable uint32_t m_visited_quiesence_nodes{0};
  Search_mode m_search_mode{Search_mode::depth};
  std::chrono::time_point<std::chrono::system_clock> m_search_start_time;
  std::chrono::time_point<std::chrono::system_clock> m_search_desired_end_time;
  std::chrono::time_point<std::chrono::system_clock> m_search_end_time;

  // How likely we think we are to win/lose to the opponent. Influences how
  // valuable a draw is. scores <0 imply we think we will win, so draws should
  // be avoided (draws are worse than an even position). 0 means equally strong
  // opponent.
  static constexpr int c_contempt_score{-10};
};
} // namespace Meneldor

#endif // MENELDOR_ENGINE
