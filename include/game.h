#ifndef GAME_H
#define GAME_H

#include "chess_types.h"

class Player;

/**
 * Allows the user to play a game of chess
 */
class Game
{
public:
  Game();

  ~Game() = default;

  Game(Game const&) = delete;
  Game(Game&&) = delete;

  Game& operator=(Game const&) = delete;
  Game& operator=(Game&&) = delete;

  bool set_starting_position(std::string fen);

  void player_vs_player();

  void player_vs_computer(Color player_color);

  void computer_vs_computer();

  // Ctrl-C to stop move (after the current player's turn) and print out moves
  // Ctrl-Z will force a stop, but the moves will not be printed out
  void play_game(Player& white_player, Player& black_player);

private:
  void init_handler_();

  std::string m_starting_position_fen{"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"};
};
#endif
