#include "move.h"

std::ostream& operator<<(std::ostream& os, Move const& self)
{
  // Output move in uci format (e.g. e7e5, or a2a1q for promotions)
  os << self.from() << self.to();
  if (self.promotion() != Piece::empty)
  {
    std::stringstream ss;
    ss << self.promotion();
    os << static_cast<char>(tolower(ss.str().front()));
  }

  return os;
}
