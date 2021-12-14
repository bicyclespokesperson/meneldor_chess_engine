#include "bitboard.h"

std::ostream& operator<<(std::ostream& os, Bitboard const& self)
{
  for (int y{7}; y >= 0; --y)
  {
    for (int x{0}; x < 8; ++x)
    {
      char result = self.is_set({x, y}) ? '1' : '.';
      os << result << ' ';
    }
    os << "\n";
  }
  os << self.hex_str() << "\n";
  return os;
}
