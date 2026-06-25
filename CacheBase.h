#pragma once

#include <cstdint>
#include <vector>

#include "Const.h"
#include "IMemory.h"

class CacheBase : public IMemory {
 public:
  explicit CacheBase(IMemory& memory);

  uint32_t LoadWord(uint32_t address, bool is_instr = false) override;
  uint16_t LoadHalf(uint32_t address) override;
  uint8_t LoadByte(uint32_t address) override;

  void StoreWord(uint32_t address, uint32_t value) override;
  void StoreHalf(uint32_t address, uint32_t value) override;
  void StoreByte(uint32_t address, uint32_t value) override;
  void ResetStats();

  uint64_t get_instr_acc() const;
  uint64_t get_instr_hit() const;
  uint64_t get_data_acc() const;
  uint64_t get_data_hit() const;

 protected:
  struct Line {
    bool valid;
    bool dirty;
    uint32_t tag;
    std::vector<uint8_t> data;
  };

  struct Address {
    uint32_t tag;
    uint32_t index;
    uint32_t offset;
  };

  struct Set {
    std::vector<Line> lines;
  };

  std::vector<Set> sets_;

  virtual uint8_t ChooseNext(uint32_t index) = 0;
  virtual void MarkUsed(uint32_t index, uint8_t way) = 0;

 private:
  uint64_t instr_hit_;
  uint64_t instr_access_;
  uint64_t data_hit_;
  uint64_t data_access_;
  IMemory& memory_;

  int8_t FindHit(Set& set, uint32_t tag) const;
  void UpdateStats(bool is_instr, bool hit);
  
  Address DecodeAddress(uint32_t address) const;
  uint32_t EncodeAddress(const Address& address) const;

  void WriteBack(uint32_t set_index, uint8_t way);
  void LoadNewLine(uint32_t address, uint32_t set_index, uint8_t way);

  void AccessStore(uint32_t addr, uint32_t size, uint32_t value, bool is_instr);
  uint32_t AccessLoad(uint32_t addr, uint32_t size, bool is_instr);

  void StoreLineData(Line& line, uint32_t offset, uint32_t value, uint8_t size);
  uint32_t LoadLineData(Line& line, uint32_t offset, uint8_t size);

  Line& GetLine(uint32_t addr, bool is_instr);
  Line& PrepareIfHit(Set& set, uint32_t index, uint8_t way, bool is_instr);
  Line& PrepareIfMiss(Set& set, uint32_t index, uint32_t address, bool is_instr);
};
