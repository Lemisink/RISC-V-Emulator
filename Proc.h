#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "IMemory.h"

class Proc {
 public:
  explicit Proc(IMemory& memory);

  // Public helpers
  void set_pc(uint32_t pc);
  void set_reg(uint8_t index, uint32_t value);
  void add_memory(uint32_t addres, const std::vector<uint8_t> buffer);
  
  bool check_done();
  uint32_t get_pc() const;
  uint32_t get_reg(uint8_t index) const;

  // Load
  void lw(uint32_t rd, uint32_t offset, uint32_t rs);
  void lb(uint32_t rd, uint32_t offset, uint32_t rs);
  void lh(uint32_t rd, uint32_t offset, uint32_t rs);
  void lbu(uint32_t rd, uint32_t offset, uint32_t rs);
  void lhu(uint32_t rd, uint32_t offset, uint32_t rs);

  // Store
  void sb(uint32_t rs1, uint32_t offset, uint32_t rs2);
  void sh(uint32_t rs1, uint32_t offset, uint32_t rs2);
  void sw(uint32_t rs1, uint32_t offset, uint32_t rs2);

  // Arithmetic/logic/shifts imm
  void addi(uint32_t rd, uint32_t rs, uint32_t imm);
  void slti(uint32_t rd, uint32_t rs, uint32_t imm);
  void sltiu(uint32_t rd, uint32_t rs, uint32_t imm);
  void andi(uint32_t rd, uint32_t rs, uint32_t imm);
  void ori(uint32_t rd, uint32_t rs, uint32_t imm);
  void xori(uint32_t rd, uint32_t rs, uint32_t imm);
  void slli(uint32_t rd, uint32_t rs, uint32_t imm);
  void srli(uint32_t rd, uint32_t rs, uint32_t imm);
  void srai(uint32_t rd, uint32_t rs, uint32_t imm);

  // Arithmetic/logic/shifts reg-reg
  void add(uint32_t rd, uint32_t rs1, uint32_t rs2);
  void sub(uint32_t rd, uint32_t rs1, uint32_t rs2);
  void slt(uint32_t rd, uint32_t rs1, uint32_t rs2);
  void sltu(uint32_t rd, uint32_t rs1, uint32_t rs2);
  void andr(uint32_t rd, uint32_t rs1, uint32_t rs2);
  void orr(uint32_t rd, uint32_t rs1, uint32_t rs2);
  void xorr(uint32_t rd, uint32_t rs1, uint32_t rs2);
  void sll(uint32_t rd, uint32_t rs1, uint32_t rs2);
  void srl(uint32_t rd, uint32_t rs1, uint32_t rs2);
  void sra(uint32_t rd, uint32_t rs1, uint32_t rs2);

  // Top bits
  void lui(uint32_t rd, uint32_t imm);
  void auipc(uint32_t rd, uint32_t imm);

  // Branching
  void beq(uint32_t rs1, uint32_t rs2, uint32_t offset);
  void bne(uint32_t rs1, uint32_t rs2, uint32_t offset);
  void blt(uint32_t rs1, uint32_t rs2, uint32_t offset);
  void bge(uint32_t rs1, uint32_t rs2, uint32_t offset);
  void bltu(uint32_t rs1, uint32_t rs2, uint32_t offset);
  void bgeu(uint32_t rs1, uint32_t rs2, uint32_t offset);

  // Jump
  void jalr(uint32_t rd, uint32_t offset, uint32_t rs);
  void jal(uint32_t rd, uint32_t offset);

  // RV32M
  void mul(uint32_t rd, uint32_t rs1, uint32_t rs2);
  void mulh(uint32_t rd, uint32_t rs1, uint32_t rs2);
  void mulhsu(uint32_t rd, uint32_t rs1, uint32_t rs2);
  void mulhu(uint32_t rd, uint32_t rs1, uint32_t rs2);
  void div(uint32_t rd, uint32_t rs1, uint32_t rs2);
  void divu(uint32_t rd, uint32_t rs1, uint32_t rs2);
  void rem(uint32_t rd, uint32_t rs1, uint32_t rs2);
  void remu(uint32_t rd, uint32_t rs1, uint32_t rs2);

  // System
  void ebreak();
  void ecall();
  void fence();

 private:
  // Private helpers
  void assign_reg(uint32_t rd, uint32_t value);
  void add2pc(uint32_t offset);

  // Varibles
  uint32_t pc_ = 0;
  uint32_t ret_addr_ = -1;
  uint32_t regs_[32];
  bool stop_ = false;
  bool started_ = false;
  IMemory& mem_;
};
