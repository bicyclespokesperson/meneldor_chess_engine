#ifndef UCI_ENGINE_PLAYER_H
#define UCI_ENGINE_PLAYER_H

#include "player.h"

namespace Meneldor
{
class Uci_engine_player : public Player
{
#ifdef _WIN32

  Uci_engine_player(std::string name, std::filesystem::path /* engine_path */, int /* depth */)
  : Player(std::move(name))
  {
    MY_ASSERT(false, "Uci_engine_player not supported on Windows");
  }
#else
public:
  static std::unique_ptr<Uci_engine_player> create(std::filesystem::path const& name, int search_depth);

  Uci_engine_player(std::string name, std::filesystem::path engine_path, int depth);

  ~Uci_engine_player() override;

  std::optional<std::string> get_next_move(std::istream& in, std::ostream& out) override;

  void notify(std::string const& move) override;

  bool set_position(std::string_view fen) override;

  void reset() override;

private:
  static inline std::filesystem::path const c_engine_binary_dir{"/Users/jeremysigrist/Desktop/chess_engine_binaries"};

  bool send_message_(std::string_view msg);
  std::string receive_message_();
  void terminate_engine_process_();
  bool init_engine_();

  std::filesystem::path m_engine_path{};
  Board m_board;
  int m_search_depth;

  std::array<int, 2> m_to_child{};
  std::array<int, 2> m_from_child{};
  pid_t m_child_pid{};
#endif
};
} // namespace Meneldor
#endif // UCI_ENGINE_PLAYER_H
