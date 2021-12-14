#include "player.h"
#include "board.h"
#include "coordinates.h"
#include "utils.h"

Player::Player(std::string name) : m_name(std::move(name))
{
}

std::string const& Player::get_name() const
{
  return m_name;
}
