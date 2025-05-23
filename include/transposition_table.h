#ifndef TRANSPOSITION_TABLE_H
#define TRANSPOSITION_TABLE_H

#include "zobrist_hash.h"

namespace Meneldor
{
class Transposition_table
{
public:
  enum class Eval_type : uint8_t
  {
    alpha = 0,
    beta,
    exact
  };

  struct Entry
  {
    zhash_t key{0};
    int depth{0};
    int evaluation{0};
    Move best_move{};
    Eval_type type{Eval_type::alpha};
  };

  static_assert(sizeof(Entry) == 24);

  Transposition_table(size_t table_size_bytes);

  ~Transposition_table() = default;

  void insert(zhash_t h, Entry entry);

  Entry const* get(zhash_t key) const;

  size_t get_capacity() const;

  // Returns the number of non-null entries in the table
  size_t count() const;

  void clear();

private:
  // Store two entries per hash value, largest depth and newest
  static constexpr size_t c_entries_per_key{2};

  Entry* walk_(zhash_t key);
  Entry const* walk_(zhash_t key) const;

  size_t hash_fn_(zhash_t key) const;

  size_t const m_table_capacity{0};
  std::vector<Entry> m_table;
};
} // namespace Meneldor

#endif // TRANSPOSITION_TABLE_H
