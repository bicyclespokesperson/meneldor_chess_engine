// Compile command: gcc -O3 -w -o find_magics find_magics.c
// From: https://www.chessprogramming.org/Looking_for_Magics

#include <stdio.h>
#include <stdlib.h>

#define USE_32_BIT_MULTIPLICATIONS

typedef unsigned long long uint64;

uint64 random_uint64()
{
  uint64 u1, u2, u3, u4;
  u1 = (uint64)(random()) & 0xFFFF;
  u2 = (uint64)(random()) & 0xFFFF;
  u3 = (uint64)(random()) & 0xFFFF;
  u4 = (uint64)(random()) & 0xFFFF;
  return u1 | (u2 << 16) | (u3 << 32) | (u4 << 48);
}

uint64 random_uint64_fewbits()
{
  return random_uint64() & random_uint64() & random_uint64();
}

int count_1s(uint64 b)
{
  int r;
  for (r = 0; b; r++, b &= b - 1)
    ;
  return r;
}

const int BitTable[64] = {63, 30, 3,  32, 25, 41, 22, 33, 15, 50, 42, 13, 11, 53, 19, 34, 61, 29, 2,  51, 21, 43,
                          45, 10, 18, 47, 1,  54, 9,  57, 0,  35, 62, 31, 40, 4,  49, 5,  52, 26, 60, 6,  23, 44,
                          46, 27, 56, 16, 7,  39, 48, 24, 59, 14, 12, 55, 38, 28, 58, 20, 37, 17, 36, 8};

int pop_1st_bit(uint64* bb)
{
  uint64 b = *bb ^ (*bb - 1);
  unsigned int fold = (unsigned)((b & 0xffffffff) ^ (b >> 32));
  *bb &= (*bb - 1);
  return BitTable[(fold * 0x783a9b23) >> 26];
}

uint64 index_to_uint64(int index, int bits, uint64 m)
{
  int i, j;
  uint64 result = 0ULL;
  for (i = 0; i < bits; i++)
  {
    j = pop_1st_bit(&m);
    if (index & (1 << i))
    {
      result |= (1ULL << j);
    }
  }
  return result;
}

uint64 rook_potential_blockers(int sq)
{
  uint64 result = 0ULL;
  int rk = sq / 8, fl = sq % 8, r, f;
  for (r = rk + 1; r <= 6; r++)
    result |= (1ULL << (fl + r * 8));
  for (r = rk - 1; r >= 1; r--)
    result |= (1ULL << (fl + r * 8));
  for (f = fl + 1; f <= 6; f++)
    result |= (1ULL << (f + rk * 8));
  for (f = fl - 1; f >= 1; f--)
    result |= (1ULL << (f + rk * 8));
  return result;
}

uint64 bishop_potential_blockers(int sq)
{
  uint64 result = 0ULL;
  int rk = sq / 8, fl = sq % 8, r, f;
  for (r = rk + 1, f = fl + 1; r <= 6 && f <= 6; r++, f++)
    result |= (1ULL << (f + r * 8));
  for (r = rk + 1, f = fl - 1; r <= 6 && f >= 1; r++, f--)
    result |= (1ULL << (f + r * 8));
  for (r = rk - 1, f = fl + 1; r >= 1 && f <= 6; r--, f++)
    result |= (1ULL << (f + r * 8));
  for (r = rk - 1, f = fl - 1; r >= 1 && f >= 1; r--, f--)
    result |= (1ULL << (f + r * 8));
  return result;
}

uint64 rook_attacked_squares(int sq, uint64 block)
{
  uint64 result = 0ULL;
  int rk = sq / 8, fl = sq % 8, r, f;
  for (r = rk + 1; r <= 7; r++)
  {
    result |= (1ULL << (fl + r * 8));
    if (block & (1ULL << (fl + r * 8)))
      break;
  }
  for (r = rk - 1; r >= 0; r--)
  {
    result |= (1ULL << (fl + r * 8));
    if (block & (1ULL << (fl + r * 8)))
      break;
  }
  for (f = fl + 1; f <= 7; f++)
  {
    result |= (1ULL << (f + rk * 8));
    if (block & (1ULL << (f + rk * 8)))
      break;
  }
  for (f = fl - 1; f >= 0; f--)
  {
    result |= (1ULL << (f + rk * 8));
    if (block & (1ULL << (f + rk * 8)))
      break;
  }
  return result;
}

uint64 bishop_attacked_squares(int sq, uint64 block)
{
  uint64 result = 0ULL;
  int rk = sq / 8, fl = sq % 8, r, f;
  for (r = rk + 1, f = fl + 1; r <= 7 && f <= 7; r++, f++)
  {
    result |= (1ULL << (f + r * 8));
    if (block & (1ULL << (f + r * 8)))
      break;
  }
  for (r = rk + 1, f = fl - 1; r <= 7 && f >= 0; r++, f--)
  {
    result |= (1ULL << (f + r * 8));
    if (block & (1ULL << (f + r * 8)))
      break;
  }
  for (r = rk - 1, f = fl + 1; r >= 0 && f <= 7; r--, f++)
  {
    result |= (1ULL << (f + r * 8));
    if (block & (1ULL << (f + r * 8)))
      break;
  }
  for (r = rk - 1, f = fl - 1; r >= 0 && f >= 0; r--, f--)
  {
    result |= (1ULL << (f + r * 8));
    if (block & (1ULL << (f + r * 8)))
      break;
  }
  return result;
}

int transform(uint64 blockers, uint64 magic, int bits)
{
//#if defined(USE_32_BIT_MULTIPLICATIONS)
  //return (unsigned)((int)blockers * (int)magic ^ (int)(blockers >> 32) * (int)(magic >> 32)) >> (32 - bits);
//#else
  return (int)((blockers * magic) >> (64 - bits));
//#endif
}

uint64 find_magic(int sq, int m, int bishop)
{
  uint64 blockers[4096];
  uint64 attacked[4096]; // Attacked squares, accounting for blockers 
  uint64 mask, used[4096], magic;
  int i, j, k, n, fail;

  mask = bishop ? bishop_potential_blockers(sq) : rook_potential_blockers(sq);
  n = count_1s(mask);

  for (i = 0; i < (1 << n); i++)
  {
    blockers[i] = index_to_uint64(i, n, mask); // Permutations?
    attacked[i] = bishop ? bishop_attacked_squares(sq, blockers[i]) : rook_attacked_squares(sq, blockers[i]);
  }
  for (k = 0; k < 100000000; k++)
  {
    magic = random_uint64_fewbits();
    if (count_1s((mask * magic) & 0xFF00000000000000ULL) < 6)
      continue;
    for (i = 0; i < 4096; i++)
      used[i] = 0ULL;
    for (i = 0, fail = 0; !fail && i < (1 << n); i++)
    {
      j = transform(blockers[i], magic, m);
      if (used[j] == 0ULL)
        used[j] = attacked[i];
      else // if (used[j] != attacked[i])
        fail = 1;
    }
    if (!fail)
      return magic;
  }
  printf("***Failed***\n");
  return 0ULL;
}

int RBits[64] = {12, 11, 11, 11, 11, 11, 11, 12, 11, 10, 10, 10, 10, 10, 10, 11, 11, 10, 10, 10, 10, 10,
                 10, 11, 11, 10, 10, 10, 10, 10, 10, 11, 11, 10, 10, 10, 10, 10, 10, 11, 11, 10, 10, 10,
                 10, 10, 10, 11, 11, 10, 10, 10, 10, 10, 10, 11, 12, 11, 11, 11, 11, 11, 11, 12};

int BBits[64] = {6, 5, 5, 5, 5, 5, 5, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 7, 7, 7, 7, 5, 5, 5, 5, 7, 9, 9, 7, 5, 5,
                 5, 5, 7, 9, 9, 7, 5, 5, 5, 5, 7, 7, 7, 7, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 5, 5, 5, 5, 5, 5, 6};

int main()
{
  int square;

  printf("const uint64 RMagic[64] = {\n");
  for (square = 0; square < 64; square++)
    printf("  0x%llxULL,\n", find_magic(square, 12, 0));
  printf("};\n\n");

  printf("const uint64 BMagic[64] = {\n");
  for (square = 0; square < 64; square++)
    printf("  0x%llxULL,\n", find_magic(square, 9, 1));
  printf("};\n\n");

  return 0;
}
