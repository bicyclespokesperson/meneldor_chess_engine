#include "transposition_table.h"
#include "utils.h"

size_t Transposition_table::hash_fn_(zhash_t key) const
{
  // We'd like to store two entries at each position. This
  // ensures that the indices given are always even numbers, and the odd
  // number will be the second entry for the two tiered replacement scheme
  return (key % (m_table_capacity / c_entries_per_key)) * c_entries_per_key;
}

Transposition_table::Transposition_table(size_t table_size_bytes) : m_table_capacity(table_size_bytes / sizeof(Entry))
{
  m_table.resize(m_table_capacity);
}

void Transposition_table::insert(zhash_t key, Entry entry)
{
  MY_ASSERT(hash_fn_(key) < m_table.size(), "Index out of bounds");

  // e1 > e2
  auto eval_type_stronger = [&](Eval_type e1, Eval_type e2)
  {
    if (e1 == e2)
    {
      return false;
    }

    if (e1 == Eval_type::exact)
    {
      return true;
    }

    return false;
  };

  //TODO: Profile this and see if the TT is noticeably slower
  auto should_replace = [&](Entry const& existing, Entry const& replacement)
  {
    if (eval_type_stronger(replacement.type, existing.type))
    {
      return true;
    }

    if (existing.type != Eval_type::exact)
    {
      if (replacement.depth >= existing.depth)
      {
        return true;
      }
    }

    if (replacement.type == Eval_type::exact && replacement.depth >= existing.depth)
    {
      return true;
    }

    return false;
  };

  auto const hash_value = hash_fn_(key);

  entry.evaluation = std::clamp(entry.evaluation, negative_inf + 100, positive_inf - 100);
  if (should_replace(m_table[hash_value], entry))
  {
    m_table[hash_value] = std::move(entry);
  }
  else if (entry.key != m_table[hash_value].key && should_replace(m_table[hash_value + 1], entry))
  {
    m_table[hash_value + 1] = std::move(entry);
  }
}

Transposition_table::Entry const* Transposition_table::get(zhash_t key) const
{
  MY_ASSERT(hash_fn_(key) < m_table.size(), "Index out of bounds");

  return walk_(key);
}

Transposition_table::Entry const* Transposition_table::walk_(zhash_t key) const
{
  MY_ASSERT(hash_fn_(key) < m_table.size(), "Index out of bounds");

  auto const hash_value = hash_fn_(key);
  auto const& candidate = m_table[hash_value];
  if (candidate.key == key)
  {
    return &m_table[hash_value];
  }
  if (m_table[hash_value + 1].key == key)
  {
    return &m_table[hash_value + 1];
  }
  return nullptr;
}

Transposition_table::Entry* Transposition_table::walk_(zhash_t key)
{
  // Idiom for sharing implementation between const and non-const versions of a method
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
  return const_cast<Entry*>(std::as_const(*this).walk_(key));
}

size_t Transposition_table::get_capacity() const
{
  return m_table_capacity;
}
