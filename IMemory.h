#pragma once

#include <cstdint>

class IMemory {  // The basis of memory
 public:
  virtual uint32_t LoadWord(uint32_t address, bool is_instr = false) = 0;
  virtual uint16_t LoadHalf(uint32_t address) = 0;
  virtual uint8_t LoadByte(uint32_t address) = 0;

  virtual void StoreWord(uint32_t address, uint32_t value) = 0;
  virtual void StoreHalf(uint32_t address, uint32_t value) = 0;
  virtual void StoreByte(uint32_t address, uint32_t value) = 0;
};
