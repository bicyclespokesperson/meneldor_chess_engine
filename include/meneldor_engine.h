#ifndef MENELDOR_ENGINE
#define MENELDOR_ENGINE

#include "bitboard.h"
#include "board.h"
#include "chess_types.h"
#include "coordinates.h"
#include "move_orderer.h"
#include "senjo/ChessEngine.h"
#include "transposition_table.h"

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

  bool setEngineOption(const std::string& optionName, const std::string& optionValue) override;

  void initialize() override;

  bool isInitialized() const override;

  bool setPosition(const std::string& fen, std::string* remain = nullptr) override;

  bool makeMove(const std::string& move) override;

  std::string getFEN() const override;

  void printBoard() const override;

  bool whiteToMove() const override;

  void clearSearchData() override;

  void ponderHit() override;

  bool isRegistered() const override;

  void registerLater() override;

  bool doRegistration(const std::string& name, const std::string& code) override;

  bool isCopyProtected() const override;

  bool copyIsOK() override;

  void setDebug(const bool flag) override;

  bool isDebugOn() const override;

  bool isSearching() override;

  void stopSearching() override;

  bool stopRequested() const override;

  void waitForSearchFinish() override;

  uint64_t perft(const int depth) override;

  std::pair<Move, int> search(int depth, std::vector<Move>& legal_moves);

  std::string go(const senjo::GoParams& params, std::string* ponder = nullptr) override;

  senjo::SearchStats getSearchStats() const override;

  void resetEngineStats() override;

  void showEngineStats() const override;

  int evaluate(Board const& board) const;

  std::optional<std::vector<std::string>> get_principle_variation(std::string move_str) const;

  void print_stats(std::pair<Move, int> best_move) const;

private:
  enum class Search_mode
  {
    depth = 0,
    time,
  };

  int negamax_(Board& board, int alpha, int beta, int depth_remaining);

  int quiesce_(Board const& board, int alpha, int beta) const;

  bool has_more_time_() const;
  void calc_time_for_move_(senjo::GoParams const& params);

  bool m_is_debug{false};
  std::atomic_flag m_stop_requested{false};
  std::atomic_flag m_is_searching{false};
  Board m_board;
  Move_orderer m_orderer{};
  std::vector<zhash_t> m_previous_positions;

  constexpr static int c_default_depth{6};
  int m_depth_for_current_search{c_default_depth};

  constexpr static size_t c_transposition_table_size_bytes{128UL * 1024UL * 1024UL};
  Transposition_table m_transpositions{c_transposition_table_size_bytes};

  mutable uint32_t m_visited_nodes{0};
  mutable uint32_t m_visited_quiesence_nodes{0};
  Search_mode m_search_mode{Search_mode::depth};
  std::chrono::time_point<std::chrono::system_clock> m_search_start_time;
  std::chrono::time_point<std::chrono::system_clock> m_search_end_time;
  std::chrono::duration<double> m_time_for_move{60.0};

  // How likely we think we are to win/lose to the opponent. Influences how valuable a draw is.
  // scores <0 imply we think we will win, so draws should be avoided (draws are worse than an even position).
  // 0 means equally strong opponent.
  // TODO: Not yet supported, needs to be flipped depending on if we or our opponent is playing
  static constexpr int c_contempt_score{-10};
};

#endif // MENELDOR_ENGINE
