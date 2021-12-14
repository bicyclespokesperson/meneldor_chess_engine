#ifndef USER_PLAYER_H
#define USER_PLAYER_H

#include "player.h"

class User_player : public Player
{
public:
  User_player(std::string name);

  ~User_player() override = default;

  std::optional<std::string> get_next_move(std::istream& in, std::ostream& out) override;

  void notify(std::string const& move) override;

  bool set_position(std::string_view fen) override;

  void reset() override;

private:
  Board m_board{};
};

#endif // USER_PLAYER_H
