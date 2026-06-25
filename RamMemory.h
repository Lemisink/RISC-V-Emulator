#pragma once

#include <cstdint>
#include <vector>

#include "IMemory.h"

class RamMemory : public IMemory {
 public:
  explicit RamMemory();
  
  uint32_t LoadWord(uint32_t address, bool is_instr = false) override;
  uint16_t LoadHalf(uint32_t address) override;
  uint8_t LoadByte(uint32_t address) override;

  void StoreWord(uint32_t address, uint32_t value) override;
  void StoreHalf(uint32_t address, uint32_t value) override;
  void StoreByte(uint32_t address, uint32_t value) override;

 private:
  void check(uint32_t address, uint32_t size, uint32_t checker) const;
  std::vector<uint8_t> memory_;
};
