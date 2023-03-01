#ifndef UTILS_H
#define UTILS_H

#include "move.h"

namespace Meneldor
{
template <class T>
std::string format_with_commas(T value)
{
  std::stringstream ss;
  ss.imbue(std::locale(""));
  ss << std::fixed << std::setprecision(2) << value;
  return ss.str();
}

template <typename T>
void print_vector(std::ostream& os, std::vector<T> const& v)
{
  std::ranges::copy(v, std::ostream_iterator<T>(os, " "));
  os << "\n";
}

std::string move_to_string(Move m);

std::string move_to_string_extended(Move m);

template <typename... T>
void unused(T&&...)
{
}

} // namespace Meneldor
#endif // UTILS_H
