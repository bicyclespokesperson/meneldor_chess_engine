#ifndef ENGINE_PLAYER_H
#define ENGINE_PLAYER_H

#include "player.h"

class Engine_player : public Player
{
public:
  Engine_player(std::string name);

  ~Engine_player() override = default;

  std::optional<std::string> get_next_move(std::istream& in, std::ostream& out) override;

  void notify(std::string const& move) override;

  bool set_position(std::string_view fen) override;

  void reset() override;

private:
  Meneldor_engine m_engine{};
};

#endif // ENGINE_PLAYER_H
