#include <cstdint>
#include <cstdio>
#include <system_error>
#include <vector>

#include "Const.h"
#include "Proc.h"

void ReadFile(Proc& proc, const std::string& file_name) {
  FILE* file = fopen(file_name.c_str(), "rb");
  if (!file) {
    throw std::system_error(errno, std::generic_category(), "open " + file_name);
  }

  uint32_t pc, reg;
  if (fread(&pc, 4, 1, file) == 1) {
    proc.set_pc(pc);
  } else {
    fclose(file);
    throw std::range_error("File is empty");
  }
  for (uint8_t i = 1; i < 32; i++) {
    if (fread(&reg, 4, 1, file) == 1) {
      proc.set_reg(i, reg);
    } else {
      fclose(file);
      throw std::range_error("File is too short");
    }
  }

  uint32_t address, size;
  while (fread(&address, 4, 1, file)) {
    if (fread(&size, 4, 1, file) != 1) {
      fclose(file);
      throw std::range_error("Incorrect chunk size format");
    }
    std::vector<uint8_t> buffer(size);
    if (address + size > kMemorySize) {
      fclose(file);
      throw std::range_error("Incorrect chunk size format");
    }
    if (fread(buffer.data(), 1, size, file) != size) {
      fclose(file);
      throw std::range_error("Incorrect chunk size format");
    }
    proc.add_memory(address, buffer);
  }

  fclose(file);
}
