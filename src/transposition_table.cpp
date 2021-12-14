#include "transposition_table.h"

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

void Transposition_table::insert(zhash_t key, Entry const& entry)
{
  MY_ASSERT(hash_fn_(key) < m_table.size(), "Index out of bounds");

  /*
 * Deep + Always replacement scheme
 *
 * Here we store two entries at every position, the first entry being 'replace by depth' and the second 'always replace'. 
 * So if the new entry had greater depth, we put it in the first entry. If it had smaller depth we replace the 
 * second entry without looking at it.
 * The TwoDeep scheme here:
 * https://pure.uvt.nl/ws/portalfiles/portal/1216990/Replace_ICCA_newsletter_vol_19_no_3.pdf
 */

  // If first entry depth is lower, replace and return
  auto hash_value = hash_fn_(key);
  if (m_table[hash_value].depth < entry.depth)
  {
    m_table[hash_value] = entry;
  }
  else
  {
    m_table[hash_value + 1] = entry;
  }
}

bool Transposition_table::contains(zhash_t key) const
{
  MY_ASSERT(hash_fn_(key) < m_table.size(), "Index out of bounds");

  return walk_(key) != nullptr;
}

Transposition_table::Entry const* Transposition_table::get(zhash_t key) const
{
  MY_ASSERT(hash_fn_(key) < m_table.size(), "Index out of bounds");
  if (auto result = walk_(key))
  {
    return result;
  }
  return nullptr;
}

Transposition_table::Entry const* Transposition_table::walk_(zhash_t key) const
{
  MY_ASSERT(hash_fn_(key) < m_table.size(), "Index out of bounds");

  auto const hash_value = hash_fn_(key);
  if (m_table[hash_value].key == key)
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
  //NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
  return const_cast<Entry*>(std::as_const(*this).walk_(key));
}

size_t Transposition_table::get_capacity() const
{
  return m_table_capacity;
}
