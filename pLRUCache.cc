#include "pLRUCache.h"

#include <algorithm>

pLRUCache::pLRUCache(IMemory& memory) : CacheBase(memory) {
  plru_state_.resize(kCacheSetCount);
}

void pLRUCache::MarkUsed(uint32_t index, uint8_t way) {
  auto& set = plru_state_[index];
  uint8_t node = (kCacheWay - 1 + way);
  while (node > 0) {
    bool is_right = !(node & 1);
    node = (node - 1) / 2;
    if (set[node] == is_right) set[node].flip();
  }
}

uint8_t pLRUCache::ChooseNext(uint32_t index) {
  for (uint8_t way = 0; way < kCacheWay; way++) {
    if (!sets_[index].lines[way].valid) {
      return way;
    }
  }
  const auto& state = plru_state_[index];
  uint32_t node = 0;
  uint8_t leaf = 0;
  uint8_t span = kCacheWay;
  while (span > 1) {
    span >>= 1;
    leaf += state[node] * span;
    node = 2 * node + 1 + state[node];
  }
  return leaf;
}
