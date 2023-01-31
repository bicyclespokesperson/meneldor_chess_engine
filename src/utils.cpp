#include "utils.h"

namespace Meneldor {
std::string move_to_string(Move m)
{
  std::stringstream ss;
  ss << m;
  return ss.str();
}

std::string move_to_string_extended(Move m)
{
  std::stringstream ss;
  ss << "[" << m << ", " << m.piece() << ", " << m.victim() << ", " << m.promotion() << ", "
     << static_cast<int32_t>(m.type()) << "]";
  return ss.str();
}
}
