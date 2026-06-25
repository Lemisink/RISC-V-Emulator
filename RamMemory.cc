#include "RamMemory.h"

#include <cstring>
#include <stdexcept>

#include "Const.h"

RamMemory::RamMemory() : memory_(kMemorySize, 0) {}

uint32_t RamMemory::LoadWord(uint32_t address, bool /*is_instr*/) {
  check(address, 4, 3);
  uint32_t data;
  std::memcpy(&data, &memory_[address], 4);
  return data;
}

uint16_t RamMemory::LoadHalf(uint32_t address) {
  check(address, 2, 1);
  uint16_t data;
  std::memcpy(&data, &memory_[address], 2);
  return data;
}

uint8_t RamMemory::LoadByte(uint32_t address) {
  check(address, 1, 0);
  uint8_t data;
  std::memcpy(&data, &memory_[address], 1);
  return data;
}

void RamMemory::StoreWord(uint32_t address, uint32_t value) {
  check(address, 4, 3);
  std::memcpy(&memory_[address], &value, 4);
}

void RamMemory::StoreHalf(uint32_t address, uint32_t value) {
  check(address, 2, 1);
  std::memcpy(&memory_[address], &value, 2);
}

void RamMemory::StoreByte(uint32_t address, uint32_t value) {
  check(address, 1, 0);
  std::memcpy(&memory_[address], &value, 1);
}

void RamMemory::check(uint32_t address, uint32_t size, uint32_t checker) const {
  if (address + size > kMemorySize) {
    throw std::out_of_range("Memory access out of range");
  } else if (address & checker) {
    throw std::logic_error("Unaligned memory access");
  }
}
