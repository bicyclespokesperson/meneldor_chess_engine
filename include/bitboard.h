#ifndef BITBOARD_H
#define BITBOARD_H

#include "coordinates.h"

struct Bitboard_iterator;

struct Bitboard;

struct Bitboard
{
  using Iterator = Bitboard_iterator;

  constexpr Bitboard() = default;

  constexpr explicit Bitboard(uint64_t value) : val(value)
  {
  }

  constexpr void set_square(Coordinates coords)
  {
    val |= (uint64_t{0x01} << coords.square_index());
  }

  constexpr void set_square(size_t index)
  {
    val |= (uint64_t{0x01} << index);
  }

  constexpr void unset_square(size_t index)
  {
    val &= ~(uint64_t{0x01} << index);
  }

  constexpr void unset_square(Coordinates coords)
  {
    unset_square(coords.square_index());
  }

  constexpr void unset_all()
  {
    val ^= val;
  }

  constexpr bool is_set(Coordinates coords) const
  {
    return static_cast<bool>(val & (uint64_t{0x01} << coords.square_index()));
  }

  constexpr Bitboard operator&(Bitboard other) const
  {
    return Bitboard(this->val & other.val);
  }

  constexpr Bitboard operator|(Bitboard other) const
  {
    return Bitboard(this->val | other.val);
  }

  constexpr Bitboard operator<<(int32_t value) const
  {
    return Bitboard(this->val << value);
  }

  constexpr Bitboard operator>>(int32_t value) const
  {
    return Bitboard(this->val >> value);
  }

  constexpr Bitboard operator~() const
  {
    return Bitboard(~this->val);
  }

  constexpr Bitboard operator^(Bitboard const& other) const
  {
    return Bitboard(this->val ^ other.val);
  }

  constexpr Bitboard operator*(Bitboard const& other) const
  {
    return Bitboard(this->val * other.val);
  }

  constexpr bool operator==(Bitboard const& other) const
  {
    return val == other.val;
  }

  constexpr Bitboard& operator<<=(int32_t value)
  {
    this->val <<= value;
    return *this;
  }

  constexpr Bitboard& operator>>=(int32_t value)
  {
    this->val >>= value;
    return *this;
  }

  constexpr Bitboard& operator&=(Bitboard other)
  {
    this->val &= other.val;
    return *this;
  }

  constexpr Bitboard& operator*=(uint64_t value)
  {
    this->val *= value;
    return *this;
  }

  constexpr Bitboard& operator|=(Bitboard other)
  {
    this->val |= other.val;
    return *this;
  }

  constexpr Bitboard& operator^=(Bitboard other)
  {
    this->val ^= other.val;
    return *this;
  }

  /*
   * Return the number of one bits in the number
   */
  constexpr int32_t occupancy() const
  {
    // Kernighan's algorithm
    int32_t count = 0;
    uint64_t value{this->val};

    while (value)
    {
      ++count;
      value &= value - 1; // reset LS1B
    }
    return count;
  }

  constexpr bool is_empty() const
  {
    return val == 0;
  }

  constexpr int32_t pop_first_bit()
  {
    auto const result = bitscan_forward();
    val &= val - 1;

    return result;
  }

  constexpr int32_t bitscan_forward() const
  {
    if (val == 0)
    {
      return -1;
    }
    return __builtin_ffsll(val) - 1;
  }

  constexpr int bitscan_reverse()
  {
    if (val == 0)
    {
      return -1;
    }
    return 63 - __builtin_clzll(val);
  }

  std::string hex_str() const
  {
    std::stringstream ss;
    constexpr int chars_per_byte{2};
    ss << "0x" << std::hex << std::setfill('0') << std::setw(sizeof(uint64_t) * chars_per_byte) << val;
    return ss.str();
  }

  constexpr Iterator begin() const;

  constexpr Iterator end() const;

  /*
   * A bitboard is stored with LSB = a1, up to MSB = h8
   * Order: a1, b1, c1, ..., h1, a2, b2, ... h8
   */
  uint64_t val{0};
};

struct Bitboard_iterator
{
  using iterator_category = std::input_iterator_tag;
  using difference_type = int32_t;
  using value_type = int32_t;
  using pointer = int32_t*;
  using reference = int32_t const&;

  constexpr Bitboard_iterator(Bitboard bb) : m_bitboard(bb)
  {
    operator++();
  }

  constexpr Bitboard_iterator() = default;

  constexpr reference operator*() const
  {
    return m_val;
  }

  // Prefix increment
  constexpr Bitboard_iterator& operator++()
  {
    m_val = m_bitboard.bitscan_forward();
    m_bitboard.val &= m_bitboard.val - 1;

    return *this;
  }

  // Postfix increment
  constexpr Bitboard_iterator operator++(int)
  {
    Bitboard_iterator tmp = *this;
    ++(*this);
    return tmp;
  }

  constexpr bool operator==(const Bitboard_iterator& other) const
  {
    return m_bitboard == other.m_bitboard && m_val == other.m_val;
  }

  constexpr bool operator!=(const Bitboard_iterator& other) const
  {
    return !(*this == other);
  }

private:
  Bitboard m_bitboard{};
  int32_t m_val{-1};
};

constexpr Bitboard::Iterator Bitboard::begin() const
{
  return {*this};
}

constexpr Bitboard::Iterator Bitboard::end() const
{
  return {Bitboard{}};
}

std::ostream& operator<<(std::ostream& os, Bitboard const& self);

#endif // BITBOARD_H
