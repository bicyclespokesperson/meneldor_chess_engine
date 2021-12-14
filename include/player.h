#ifndef PLAYER_H
#define PLAYER_H

#include "board.h"
#include "chess_types.h"
#include "meneldor_engine.h"

/**
 * A player class represents one of the players in a chess game. It
 * can keep track of its pieces and make a move.
 */
class Player
{
public:
  virtual ~Player() = default;

  std::string const& get_name() const;

  // Called when a move is made successfully, either by this player or the opponent
  // "move" is a string in UCI format ("e2e4" or "a7a8q")
  virtual void notify(std::string const& move) = 0;

  // Get the player's next move
  // Return should be a string in UCI format ("e2e4" or "a7a8q")
  // Return empty if the player resigns
  virtual std::optional<std::string> get_next_move(std::istream& in, std::ostream& out) = 0;

  // Set the starting position for the game
  // Return false if the fen string could not be parsed
  virtual bool set_position(std::string_view fen) = 0;

  // Called at the start of a new game
  virtual void reset() = 0;

protected:
  Player(std::string name);

private:
  std::string m_name;
};

#endif
