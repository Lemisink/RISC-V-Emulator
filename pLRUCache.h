#pragma once

#include <bitset>
#include <vector>

#include "CacheBase.h"

class pLRUCache : public CacheBase {
 public:
  explicit pLRUCache(IMemory& memory);

 private:
  std::vector<std::bitset<kCacheWay - 1>> plru_state_;

  void MarkUsed(uint32_t index, uint8_t way) override;
  uint8_t ChooseNext(uint32_t index) override;
};
