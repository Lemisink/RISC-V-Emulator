#pragma once

#include <array>
#include <vector>

#include "CacheBase.h"

class LRUCache : public CacheBase {
 public:
  explicit LRUCache(IMemory& memory);

 private:
  std::vector<std::array<uint64_t, kCacheWay>> last_used_;
  uint64_t global_clock_;

  void MarkUsed(uint32_t index, uint8_t way) override;
  uint8_t ChooseNext(uint32_t index) override;
};
