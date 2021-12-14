#ifndef TRANSPOSITION_TABLE_H
#define TRANSPOSITION_TABLE_H

#include "zobrist_hash.h"

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

  void insert(zhash_t h, Entry const& entry);

  bool contains(zhash_t key) const;

  Entry const* get(zhash_t key) const;

  size_t get_capacity() const;

  //Debugging
  size_t count() const
  {
    return std::count_if(m_table.cbegin(), m_table.cend(),
                         [](const auto& entry)
                         {
                           return entry.best_move.type() != Move_type::null;
                         });
  }

  void display(std::ostream& out)
  {
    for (auto const& entry : m_table)
    {
      if (entry.best_move.type() != Move_type::null)
      {
        out << "depth: " << entry.depth << ", eval: " << entry.evaluation << ", move: " << entry.best_move << "\n";
      }
    }
  }

private:
  // Store two entries per hash value, largest depth and newest
  static constexpr size_t c_entries_per_key{2};

  Entry* walk_(zhash_t key);
  Entry const* walk_(zhash_t key) const;

  size_t hash_fn_(zhash_t key) const;

  const size_t m_table_capacity{0};
  std::vector<Entry> m_table;
};

#endif // TRANSPOSITION_TABLE_H
