#include "utils.h"

std::string move_to_string(Move m)
{
  std::stringstream ss;
  ss << m;
  return ss.str();
}
