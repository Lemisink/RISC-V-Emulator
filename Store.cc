#include "Store.h"

#include <cstdio>
#include <stdexcept>
#include <system_error>

#include "Const.h"

void WriteFile(Proc& proc, IMemory& memory, const std::string& file_name, uint32_t start, uint32_t size) {
  FILE* file = std::fopen(file_name.c_str(), "wb");
  if (!file) {
    throw std::system_error(errno, std::generic_category(), "open " + file_name);
  }

  if (start + size > kMemorySize) {
    std::fclose(file);
    throw std::range_error("output range is out of memory bounds");
  }
  const uint32_t pc = proc.get_pc();
  if (std::fwrite(&pc, 4, 1, file) != 1) {
    std::fclose(file);
    throw std::range_error("write pc");
  }
  for (int i = 1; i < 32; i++) {
    const uint32_t reg = proc.get_reg(i);
    if (std::fwrite(&reg, 4, 1, file) != 1) {
      std::fclose(file);
      throw std::range_error("write reg");
    }
  }

  if (std::fwrite(&start, 4, 1, file) != 1 ||
      std::fwrite(&size, 4, 1, file) != 1) {
    std::fclose(file);
    throw std::range_error("write header");
  }

  for (uint32_t i = 0; i < size; i++) {
    uint8_t byte = memory.LoadByte(start + i);
    if (std::fwrite(&byte, 1, 1, file) != 1) {
      std::fclose(file);
      throw std::range_error("write data");
    }
  }

  std::fclose(file);
}
