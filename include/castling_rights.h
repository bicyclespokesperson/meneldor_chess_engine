#ifndef CASTLING_RIGHTS_H
#define CASTLING_RIGHTS_H

using Castling_rights = uint8_t;

constexpr Castling_rights c_castling_rights_all{0x0f};
constexpr Castling_rights c_castling_rights_none{0x00};

constexpr uint8_t c_white_short_castle{0x01};
constexpr uint8_t c_white_long_castle{0x02};
constexpr uint8_t c_black_short_castle{0x04};
constexpr uint8_t c_black_long_castle{0x08};

constexpr bool white_can_short_castle(Castling_rights rights)
{
  return rights & c_white_short_castle;
}
constexpr bool white_can_long_castle(Castling_rights rights)
{
  return rights & c_white_long_castle;
}
constexpr bool black_can_short_castle(Castling_rights rights)
{
  return rights & c_black_short_castle;
}
constexpr bool black_can_long_castle(Castling_rights rights)
{
  return rights & c_black_long_castle;
}

constexpr void set_white_short_castle_false(Castling_rights& rights)
{
  rights &= ~c_white_short_castle;
}
constexpr void set_white_long_castle_false(Castling_rights& rights)
{
  rights &= ~c_white_long_castle;
}
constexpr void set_black_short_castle_false(Castling_rights& rights)
{
  rights &= ~c_black_short_castle;
}
constexpr void set_black_long_castle_false(Castling_rights& rights)
{
  rights &= ~c_black_long_castle;
}

constexpr void set_white_short_castle_true(Castling_rights& rights)
{
  rights |= c_white_short_castle;
}
constexpr void set_white_long_castle_true(Castling_rights& rights)
{
  rights |= c_white_long_castle;
}
constexpr void set_black_short_castle_true(Castling_rights& rights)
{
  rights |= c_black_short_castle;
}
constexpr void set_black_long_castle_true(Castling_rights& rights)
{
  rights |= c_black_long_castle;
}

#endif // CASTLING_RIGHTS_H
