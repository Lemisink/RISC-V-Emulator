#include "Proc.h"

#include <algorithm>
#include <bit>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <stdexcept>

#include "Const.h"
#include "RamMemory.h"

Proc::Proc(IMemory& memory)
    : pc_(0),
      ret_addr_(static_cast<uint32_t>(-1)),
      stop_(false),
      started_(false),
      mem_(memory) {
  std::fill(std::begin(regs_), std::end(regs_), 0);
}

uint32_t Proc::get_reg(uint8_t index) const {
  if (index > 31) {
    throw std::range_error("index reg too high");
  }
  return regs_[index];
}

void Proc::set_reg(uint8_t index, uint32_t value) {
  if (index != 0 && index < 32) {
    regs_[index] = value;
  }
  if (index == 1) {
    ret_addr_ = value;
  }
}

void Proc::assign_reg(uint32_t rd, uint32_t value) {
  if (rd != 0) {
    regs_[rd] = value;
  } else {
    return;
  }
}

void Proc::add2pc(uint32_t offset) {
  if ((offset & 3) == 0) {
    pc_ += offset;
  } else {
    throw std::logic_error("PC offset not 4-byte aligned");
  }
}

void Proc::set_pc(uint32_t pc) {
  pc_ = pc;
}

void Proc::add_memory(uint32_t addres, const std::vector<uint8_t> buffer) {
  for (size_t i = 0; i < buffer.size(); i++) {
    mem_.StoreByte(addres + i, buffer[i]);
  }
}

uint32_t Proc::get_pc() const { return pc_; }

bool Proc::check_done() {
  if (stop_) {
    return true;
  }
  if (!started_) {
    started_ = true;
    return false;
  }
  return pc_ == ret_addr_;
}

void Proc::lw(uint32_t rd, uint32_t offset, uint32_t rs) {
  uint32_t addres = regs_[rs] + offset;
  assign_reg(rd, mem_.LoadWord(addres));
  pc_ += 4;
}

void Proc::lb(uint32_t rd, uint32_t offset, uint32_t rs) {
  uint32_t addres = offset + regs_[rs];
  int32_t result = static_cast<int8_t>(mem_.LoadByte(addres));
  assign_reg(rd, static_cast<uint32_t>(result));
  pc_ += 4;
}

void Proc::lh(uint32_t rd, uint32_t offset, uint32_t rs) {
  uint32_t addres = offset + regs_[rs];
  int32_t result = static_cast<int16_t>(mem_.LoadHalf(addres));
  assign_reg(rd, static_cast<uint32_t>(result));
  pc_ += 4;
}

void Proc::lbu(uint32_t rd, uint32_t offset, uint32_t rs) {
  uint32_t addres = offset + regs_[rs];
  uint32_t result = mem_.LoadByte(addres);
  assign_reg(rd, result);
  pc_ += 4;
}

void Proc::lhu(uint32_t rd, uint32_t offset, uint32_t rs) {
  uint32_t addres = offset + regs_[rs];
  uint32_t result = mem_.LoadHalf(addres);
  assign_reg(rd, result);
  pc_ += 4;
}

void Proc::sb(uint32_t rs1, uint32_t offset, uint32_t rs2) {
  uint32_t addres = regs_[rs2] + offset;
  mem_.StoreByte(addres, regs_[rs1]);
  pc_ += 4;
}

void Proc::sh(uint32_t rs1, uint32_t offset, uint32_t rs2) {
  uint32_t addres = regs_[rs2] + offset;
  mem_.StoreHalf(addres, regs_[rs1]);
  pc_ += 4;
}

void Proc::sw(uint32_t rs1, uint32_t offset, uint32_t rs2) {
  uint32_t addres = regs_[rs2] + offset;
  mem_.StoreWord(addres, regs_[rs1]);
  pc_ += 4;
}

void Proc::addi(uint32_t rd, uint32_t rs, uint32_t imm) {
  uint32_t result = regs_[rs] + imm;
  assign_reg(rd, result);
  pc_ += 4;
}

void Proc::slti(uint32_t rd, uint32_t rs, uint32_t imm) {
  uint32_t result =
      (static_cast<int32_t>(imm) > static_cast<int32_t>(regs_[rs]));
  assign_reg(rd, result);
  pc_ += 4;
}

void Proc::sltiu(uint32_t rd, uint32_t rs, uint32_t imm) {
  uint32_t result = (imm > regs_[rs]);
  assign_reg(rd, result);
  pc_ += 4;
}

void Proc::andi(uint32_t rd, uint32_t rs, uint32_t imm) {
  uint32_t result = (regs_[rs] & imm);
  assign_reg(rd, result);
  pc_ += 4;
}

void Proc::ori(uint32_t rd, uint32_t rs, uint32_t imm) {
  uint32_t result = (regs_[rs] | imm);
  assign_reg(rd, result);
  pc_ += 4;
}

void Proc::xori(uint32_t rd, uint32_t rs, uint32_t imm) {
  uint32_t result = (regs_[rs] ^ imm);
  assign_reg(rd, result);
  pc_ += 4;
}

void Proc::slli(uint32_t rd, uint32_t rs, uint32_t imm) {
  uint32_t shift = (imm & 0x1f);
  uint32_t result = (regs_[rs] << shift);
  assign_reg(rd, result);
  pc_ += 4;
}

void Proc::srli(uint32_t rd, uint32_t rs, uint32_t imm) {
  uint32_t shift = (imm & 0x1f);
  uint32_t result = (regs_[rs] >> shift);
  assign_reg(rd, result);
  pc_ += 4;
}

void Proc::srai(uint32_t rd, uint32_t rs, uint32_t imm) {
  uint32_t shift = (imm & 0x1f);
  uint32_t result = (static_cast<int32_t>(regs_[rs]) >> shift);
  assign_reg(rd, result);
  pc_ += 4;
}

void Proc::add(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  uint32_t result = regs_[rs1] + regs_[rs2];
  assign_reg(rd, result);
  pc_ += 4;
}

void Proc::sub(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  uint32_t result = regs_[rs1] - regs_[rs2];
  assign_reg(rd, result);
  pc_ += 4;
}

void Proc::slt(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  uint32_t result =
      (static_cast<int32_t>(regs_[rs2]) > static_cast<int32_t>(regs_[rs1]));
  assign_reg(rd, result);
  pc_ += 4;
}

void Proc::sltu(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  uint32_t result = (regs_[rs2] > regs_[rs1]);
  assign_reg(rd, result);
  pc_ += 4;
}

void Proc::andr(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  uint32_t result = (regs_[rs1] & regs_[rs2]);
  assign_reg(rd, result);
  pc_ += 4;
}

void Proc::orr(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  uint32_t result = (regs_[rs1] | regs_[rs2]);
  assign_reg(rd, result);
  pc_ += 4;
}

void Proc::xorr(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  uint32_t result = (regs_[rs1] ^ regs_[rs2]);
  assign_reg(rd, result);
  pc_ += 4;
}

void Proc::sll(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  uint32_t shift = regs_[rs2] & 0x1f;
  uint32_t result = (regs_[rs1] << shift);
  assign_reg(rd, result);
  pc_ += 4;
}

void Proc::srl(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  uint32_t shift = regs_[rs2] & 0x1f;
  uint32_t result = (regs_[rs1] >> shift);
  assign_reg(rd, result);
  pc_ += 4;
}

void Proc::sra(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  uint32_t shift = regs_[rs2] & 0x1f;
  uint32_t result = (static_cast<int32_t>(regs_[rs1]) >> shift);
  assign_reg(rd, result);
  pc_ += 4;
}

void Proc::lui(uint32_t rd, uint32_t imm) {
  uint32_t result = (imm << 12);
  assign_reg(rd, result);
  pc_ += 4;
}

void Proc::auipc(uint32_t rd, uint32_t imm) {
  uint32_t result = pc_ + (imm << 12);
  assign_reg(rd, result);
  pc_ += 4;
}

void Proc::beq(uint32_t rs1, uint32_t rs2, uint32_t offset) {
  bool result = (regs_[rs1] == regs_[rs2]);
  uint32_t res_offset = offset * result + 4 * (!result);
  add2pc(res_offset);
}

void Proc::bne(uint32_t rs1, uint32_t rs2, uint32_t offset) {
  bool result = (regs_[rs1] != regs_[rs2]);
  uint32_t res_offset = offset * result + 4 * (!result);
  add2pc(res_offset);
}

void Proc::blt(uint32_t rs1, uint32_t rs2, uint32_t offset) {
  bool result =
      (static_cast<int32_t>(regs_[rs2]) > static_cast<int32_t>(regs_[rs1]));
  uint32_t res_offset = offset * result + 4 * (!result);
  add2pc(res_offset);
}

void Proc::bge(uint32_t rs1, uint32_t rs2, uint32_t offset) {
  bool result =
      (static_cast<int32_t>(regs_[rs1]) >= static_cast<int32_t>(regs_[rs2]));
  uint32_t res_offset = offset * result + 4 * (!result);
  add2pc(res_offset);
}

void Proc::bltu(uint32_t rs1, uint32_t rs2, uint32_t offset) {
  bool result = regs_[rs2] > regs_[rs1];
  uint32_t res_offset = offset * result + 4 * (!result);
  add2pc(res_offset);
}

void Proc::bgeu(uint32_t rs1, uint32_t rs2, uint32_t offset) {
  bool result = (regs_[rs1] >= regs_[rs2]);
  uint32_t res_offset = offset * result + 4 * (!result);
  add2pc(res_offset);
}

void Proc::jal(uint32_t rd, uint32_t offset) {
  assign_reg(rd, pc_ + 4);
  add2pc(offset);
}

void Proc::jalr(uint32_t rd, uint32_t offset, uint32_t rs) {
  assign_reg(rd, pc_ + 4);
  uint32_t result = (offset + regs_[rs]) & (~1);
  add2pc(result - pc_);
}

void Proc::mul(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  uint64_t result =
      static_cast<int64_t>(regs_[rs1]) * static_cast<int64_t>(regs_[rs2]);
  assign_reg(rd, static_cast<uint32_t>(result));
  pc_ += 4;
}

void Proc::mulh(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  uint64_t raw_result =
      static_cast<int64_t>(regs_[rs1]) * static_cast<int64_t>(regs_[rs2]);
  uint32_t result = static_cast<uint32_t>(raw_result >> 32);
  assign_reg(rd, result);
  pc_ += 4;
}

void Proc::mulhsu(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  uint64_t raw_result =
      static_cast<int64_t>(regs_[rs1]) * static_cast<uint64_t>(regs_[rs2]);
  uint32_t result = static_cast<uint32_t>(raw_result >> 32);
  assign_reg(rd, result);
  pc_ += 4;
}

void Proc::mulhu(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  uint64_t raw_result =
      static_cast<uint64_t>(regs_[rs1]) * static_cast<uint64_t>(regs_[rs2]);
  uint32_t result = static_cast<uint32_t>(raw_result >> 32);
  assign_reg(rd, result);
  pc_ += 4;
}

void Proc::div(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  if (regs_[rs2] == 0) {
    assign_reg(rd, 0xffffffff);
  } else if (regs_[rs2] == 0xffffffff && regs_[rs1] == 0x80000000) {
    assign_reg(rd, 0x80000000);
  } else {
    uint32_t result =
        static_cast<int32_t>(regs_[rs1]) / static_cast<int32_t>(regs_[rs2]);
    assign_reg(rd, result);
  }
  pc_ += 4;
}

void Proc::divu(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  if (regs_[rs2] == 0) {
    assign_reg(rd, 0xffffffff);
  } else {
    uint32_t result = regs_[rs1] / regs_[rs2];
    assign_reg(rd, result);
  }
  pc_ += 4;
}

void Proc::rem(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  if (regs_[rs2] == 0) {
    assign_reg(rd, regs_[rs1]);
  } else if (regs_[rs1] == 0x80000000 && regs_[rs2] == 0xffffffff) {
    assign_reg(rd, 0);
  } else {
    uint32_t result =
        static_cast<int32_t>(regs_[rs1]) % static_cast<int32_t>(regs_[rs2]);
    assign_reg(rd, result);
  }
  pc_ += 4;
}

void Proc::remu(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  if (regs_[rs2] == 0) {
    assign_reg(rd, regs_[rs1]);
  } else {
    uint32_t result = regs_[rs1] % regs_[rs2];
    assign_reg(rd, result);
  }
  pc_ += 4;
}

void Proc::ebreak() {
  stop_ = true;
  pc_ += 4;
}

void Proc::ecall() {
  stop_ = true;
  pc_ += 4;
}

void Proc::fence() {
  pc_ += 4;
}
