#include "LRUCache.h"

#include <algorithm>
#include <iostream>

LRUCache::LRUCache(IMemory& memory)
    : CacheBase(memory),
      global_clock_(0) {
  last_used_.resize(kCacheSetCount);
  for (auto& set : last_used_) {
    for (auto& line : set) {
      line = 0;
    }
  }
}

void LRUCache::MarkUsed(uint32_t index, uint8_t way) {
  last_used_[index][way] = ++global_clock_;
}

uint8_t LRUCache::ChooseNext(uint32_t index) {
  for (uint8_t way = 0; way < kCacheWay; way++) {
    if (!sets_[index].lines[way].valid) {
      return way;
    }
  }
  const auto& set = last_used_[index];
  const auto it = std::min_element(set.begin(), set.end());
  return static_cast<uint8_t>(std::distance(set.begin(), it));
}
