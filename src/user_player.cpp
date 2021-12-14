#include "user_player.h"
#include "utils.h"

User_player::User_player(std::string name) : Player(std::move(name))
{
}

// Returns empty if the player resigns.
std::optional<std::string> User_player::get_next_move(std::istream& in, std::ostream& out)
{
  constexpr static bool uci_mode{false};

  std::string line = "";
  line.reserve(64);

  do
  {
    if (!in.good())
    {
      return {};
    }

    out << get_name() << ", please enter the beginning and ending squares of the move (ex: "
        << (uci_mode ? std::string{"e2e4"} : std::string{"Bxc7"}) << "): ";

    // Get move from the user and ensure that it is of the correct form
    getline(in, line);
    if (line == "quit")
    {
      // Empty if the player resigned
      return {};
    }

    if (auto move =
          uci_mode ? m_board.move_from_uci(line) : m_board.move_from_algebraic(line, m_board.get_active_color()))
    {
      return move_to_string(*move);
    }
  } while (true);
}

void User_player::notify(std::string const& move)
{
  m_board.try_move_uci(move);
}

bool User_player::set_position(std::string_view fen)
{
  if (auto board = Board::from_fen(fen))
  {
    m_board = *board;
    return true;
  }
  return false;
}

void User_player::reset()
{
  m_board = {};
}
