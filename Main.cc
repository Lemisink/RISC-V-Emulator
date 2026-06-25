#include <cstdio>
#include <exception>
#include <iostream>
#include <limits>
#include <string>

#include "Const.h"
#include "LRUCache.h"
#include "Load.h"
#include "Proc.h"
#include "RamMemory.h"
#include "Runner.h"
#include "Store.h"
#include "pLRUCache.h"

namespace {

uint32_t parse_u32(const char* s) {
  return static_cast<uint32_t>(std::stoul(s, nullptr, 0));
}

void print_row(const char* name,
               uint64_t instr_acc, uint64_t instr_hit,
               uint64_t data_acc, uint64_t data_hit) {
  const uint64_t total_acc = instr_acc + data_acc;
  if (total_acc == 0) {
    std::printf("| %-10s |       nan%% |           nan%% |          nan%% |            0 |            0 |            0 |            0 |\n", name);
    return;
  }
  const double hit_rate = 100.0 * (instr_hit + data_hit) / static_cast<double>(total_acc);
  const double instr_rate = instr_acc ? 100.0 * instr_hit / static_cast<double>(instr_acc) : std::numeric_limits<double>::quiet_NaN();
  const double data_rate = data_acc ? 100.0 * data_hit / static_cast<double>(data_acc) : std::numeric_limits<double>::quiet_NaN();
  std::printf("| %-10s | %3.4f%% |       %3.4f%% |      %3.4f%% | %12lu | %12lu | %12lu | %12lu |\n",
              name, hit_rate, instr_rate, data_rate,
              instr_acc, instr_hit, data_acc, data_hit);
}

}

int main(int argc, char** argv) {
  bool need_write = false;
  uint32_t address = 0;
  uint32_t size = 0;
  std::string output_file;
  std::string input_file;

  for (int i = 1; i < argc; i++) {
    const std::string arg = argv[i];
    if (arg == "-i") {
      if (i + 1 >= argc) {
        std::cerr << "Missing input file after -i" << std::endl;
        return 1;
      }
      input_file = argv[++i];
    } else if (arg == "-o") {
      if (i + 3 >= argc) {
        std::cerr << "Missing arguments after -o" << std::endl;
        return 1;
      }
      need_write = true;
      output_file = argv[++i];
      address = parse_u32(argv[++i]);
      size = parse_u32(argv[++i]);
    } else {
      std::cerr << "Unknown argument" << std::endl;
      return 1;
    }
  }

  if (input_file.empty()) {
    std::cerr << "Input file is required" << std::endl;
    return 1;
  }

  // Первый прогон: LRU
  RamMemory ram_lru;
  LRUCache lru_cache(ram_lru);
  Proc proc_lru(lru_cache);
  // Второй прогон: bit-pLRU
  RamMemory ram_plru;
  pLRUCache plru_cache(ram_plru);
  Proc proc_plru(plru_cache);

  try {
    ReadFile(proc_lru, input_file);
    ReadFile(proc_plru, input_file);
    lru_cache.ResetStats();
    plru_cache.ResetStats();
    Runner(proc_lru, lru_cache);
    Runner(proc_plru, plru_cache);
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  std::printf("| replacement | hit_rate | instr_hit_rate | data_hit_rate | instr_access |  instr_hit   | data_access  |   data_hit   |\n");
  std::printf("| :---------- | :------: | -------------: | ------------: | -----------: | -----------: | -----------: | -----------: |\n");
  print_row("LRU",
            lru_cache.get_instr_acc(), lru_cache.get_instr_hit(),
            lru_cache.get_data_acc(), lru_cache.get_data_hit());
  print_row("bpLRU",
            plru_cache.get_instr_acc(), plru_cache.get_instr_hit(),
            plru_cache.get_data_acc(), plru_cache.get_data_hit());

  if (need_write) {
    WriteFile(proc_lru, lru_cache, output_file, address, size);
  }
  return 0;
}
