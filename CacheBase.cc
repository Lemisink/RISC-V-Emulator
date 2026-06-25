#include "CacheBase.h"

#include <cstring>

CacheBase::CacheBase(IMemory& memory)
    : instr_hit_(0),
      instr_access_(0),
      data_hit_(0),
      data_access_(0),
      memory_(memory) {
  sets_.resize(kCacheSetCount);
  for (auto& set : sets_) {
    set.lines.resize(kCacheWay);
    for (auto& line : set.lines) {
      line.valid = false;
      line.dirty = false;
      line.tag = 0;
      line.data.assign(kCacheLineSize, 0);
    }
  }
}

void CacheBase::StoreLineData(Line& line, uint32_t offset, uint32_t value, uint8_t size) {
  std::memcpy(&line.data[offset], &value, size);
}

uint32_t CacheBase::LoadLineData(Line& line, uint32_t offset, uint8_t size) {
  uint32_t value;
  std::memcpy(&value, &line.data[offset], size);
  return value;
}

int8_t CacheBase::FindHit(Set& set, uint32_t tag) const {
  int8_t way = 0;
  for (auto& line : set.lines) {
    if (line.valid && line.tag == tag) {
      return way;
    }
    way++;
  }
  return -1;
}

void CacheBase::UpdateStats(bool is_instr, bool hit) {
  if (is_instr) {
    instr_access_++;
    instr_hit_ += hit;
  } else {
    data_access_++;
    data_hit_ += hit;
  }
}

void CacheBase::WriteBack(uint32_t set_index, uint8_t way) {
  Line& line = sets_[set_index].lines[way];
  if (!line.dirty || !line.valid) {
    return;
  }
  uint32_t line_base = (line.tag << (kCacheIndexLen + kCacheOffsetLen)) |
                       (set_index << kCacheOffsetLen);
  for (uint32_t i = 0; i < kCacheLineSize; i++) {
    memory_.StoreByte(line_base + i, line.data[i]);
  }
  line.dirty = false;
}

void CacheBase::LoadNewLine(uint32_t address, uint32_t set_index, uint8_t way) {
  Line& line = sets_[set_index].lines[way];
  const uint32_t line_base = address & ~(kCacheLineSize - 1);
  for (uint32_t i = 0; i < kCacheLineSize; i++) {
    line.data[i] = memory_.LoadByte(line_base + i);
  }
  line.dirty = false;
  line.valid = true;
  line.tag = DecodeAddress(address).tag;
}

CacheBase::Address CacheBase::DecodeAddress(const uint32_t address) const {
  constexpr uint32_t offset_mask = ~(-1 << kCacheOffsetLen);
  constexpr uint32_t index_mask = ~(-1 << kCacheIndexLen);
  const uint32_t offset = address & offset_mask;
  const uint32_t index = (address >> kCacheOffsetLen) & index_mask;
  const uint32_t tag = address >> (kCacheOffsetLen + kCacheIndexLen);
  return {tag, index, offset};
}

uint32_t CacheBase::EncodeAddress(const CacheBase::Address& address) const {
  return (address.tag << (kCacheIndexLen + kCacheOffsetLen)) |
         (address.index << kCacheOffsetLen) |
         (address.offset);
}

CacheBase::Line& CacheBase::PrepareIfHit(Set& set, uint32_t index, uint8_t way, bool is_instr) {
  MarkUsed(index, way);
  UpdateStats(is_instr, true);
  return set.lines[way];
}

CacheBase::Line& CacheBase::PrepareIfMiss(Set& set, uint32_t index, uint32_t address, bool is_instr) {
  const uint8_t victim = ChooseNext(index);
  WriteBack(index, victim); 
  LoadNewLine(address, index, victim);
  MarkUsed(index, victim);
  UpdateStats(is_instr, false);
  return set.lines[victim];
}

CacheBase::Line& CacheBase::GetLine(uint32_t addr, bool is_instr) {
  Address address = DecodeAddress(addr);
  Set& set = sets_[address.index];
  const int8_t way = FindHit(set, address.tag);
  return (way != -1)
             ? PrepareIfHit(set, address.index, way, is_instr)
             : PrepareIfMiss(set, address.index, addr, is_instr);
}

void CacheBase::AccessStore(uint32_t addr, uint32_t size, uint32_t value, bool is_instr) {
  Address address = DecodeAddress(addr);
  Line& line = GetLine(addr, is_instr);
  StoreLineData(line, address.offset, value, size);
  line.dirty = true;
}

uint32_t CacheBase::AccessLoad(uint32_t addr, uint32_t size, bool is_instr) {
  Address address = DecodeAddress(addr);
  Line& line = GetLine(addr, is_instr);
  return LoadLineData(line, address.offset, size);
}

void CacheBase::ResetStats() {
  for (uint32_t set_index = 0; set_index < (uint32_t) sets_.size(); set_index++) {
    for (uint8_t way = 0; way < kCacheWay; way++) {
      Line& line = sets_[set_index].lines[way];
      if (line.valid && line.dirty) {
        WriteBack(set_index, way);
      }
      line.valid = false;
      line.dirty = false;
    }
  }
  instr_hit_ = instr_access_ = 0;
  data_hit_ = data_access_ = 0;
}

uint64_t CacheBase::get_instr_acc() const {
  return instr_access_;
}

uint64_t CacheBase::get_instr_hit() const {
  return instr_hit_;
}

uint64_t CacheBase::get_data_acc() const {
  return data_access_;
}

uint64_t CacheBase::get_data_hit() const {
  return data_hit_;
}

uint32_t CacheBase::LoadWord(uint32_t address, bool is_instr) {
  return AccessLoad(address, 4, is_instr);
}

uint16_t CacheBase::LoadHalf(uint32_t address) {
  return static_cast<uint16_t>(AccessLoad(address, 2, false));
}

uint8_t CacheBase::LoadByte(uint32_t address) {
  return static_cast<uint8_t>(AccessLoad(address, 1, false));
}

void CacheBase::StoreWord(uint32_t address, uint32_t value) {
  AccessStore(address, 4, value, false);
}

void CacheBase::StoreHalf(uint32_t address, uint32_t value) {
  AccessStore(address, 2, value, false);
}

void CacheBase::StoreByte(uint32_t address, uint32_t value) {
  AccessStore(address, 1, value, false);
}
