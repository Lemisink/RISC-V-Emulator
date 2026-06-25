#include "Runner.h"

#include <cstdint>
#include <format>
#include <stdexcept>

#include "Proc.h"

namespace {

enum class Opcode : uint8_t {
  kLoad = 0x03,
  kImm = 0x13,
  kAuipc = 0x17,
  kStore = 0x23,
  kAlu = 0x33,
  kLui = 0x37,
  kBranch = 0x63,
  kJalr = 0x67,
  kJal = 0x6f,
  kSystem = 0x73,
  kFence = 0x0f
};

constexpr uint32_t sign_extend(uint32_t data, uint8_t bit_width) {
  return (static_cast<int32_t>(data << (32u - bit_width))) >> (32u - bit_width);
}

constexpr uint32_t get_bits(uint32_t value, uint8_t offset, uint8_t width) {
  return (value >> offset) & ((1u << width) - 1);
}

constexpr uint32_t store_offset(uint32_t instruction) {
  const uint32_t raw_result =
      (get_bits(instruction, 25, 7) << 5) | (get_bits(instruction, 7, 5));
  return sign_extend(raw_result, 12);
}

constexpr uint32_t branch_offset(uint32_t instruction) {
  const uint32_t raw_result = (get_bits(instruction, 31, 1) << 12) |
                              (get_bits(instruction, 7, 1) << 11) |
                              (get_bits(instruction, 25, 6) << 5) |
                              (get_bits(instruction, 8, 4) << 1);
  return sign_extend(raw_result, 13);
}

constexpr uint32_t jal_offset(uint32_t instruction) {
  const uint32_t raw_result = (get_bits(instruction, 31, 1) << 20) |
                              (get_bits(instruction, 12, 8) << 12) |
                              (get_bits(instruction, 20, 1) << 11) |
                              (get_bits(instruction, 21, 10) << 1);
  return sign_extend(raw_result, 21);
}

constexpr uint32_t get_offset(uint32_t instruction) {
  return sign_extend(get_bits(instruction, 20, 12), 12);
}

} // namespace

void DoInstruction(Proc& proc, uint32_t instruction);

void Runner(Proc& proc, IMemory& memory) {
  while (!proc.check_done()) {
    const uint32_t instruction = memory.LoadWord(proc.get_pc(), true);
    DoInstruction(proc, instruction);
  }
}

void DoInstruction(Proc& proc, uint32_t instruction) {
  const Opcode opcode = static_cast<Opcode>(instruction & 0x7f);
  switch (opcode) {
    case Opcode::kLoad: {
      const uint32_t funct = get_bits(instruction, 12, 3);
      const uint32_t rd = get_bits(instruction, 7, 5);
      const uint32_t rs = get_bits(instruction, 15, 5);
      const uint32_t offset = get_offset(instruction);
      switch (funct) {
        case 0x0:
          proc.lb(rd, offset, rs);
          break;
        case 0x1:
          proc.lh(rd, offset, rs);
          break;
        case 0x2:
          proc.lw(rd, offset, rs);
          break;
        case 0x4:
          proc.lbu(rd, offset, rs);
          break;
        case 0x5:
          proc.lhu(rd, offset, rs);
          break;
        default:
          throw std::invalid_argument("Invalid load funct3");
      }
      break;
    }
    case Opcode::kImm: {
      const uint32_t funct1 = get_bits(instruction, 12, 3);
      const uint32_t funct2 = get_bits(instruction, 25, 7);
      const uint32_t rd = get_bits(instruction, 7, 5);
      const uint32_t rs = get_bits(instruction, 15, 5);
      const uint32_t imm = get_offset(instruction);
      const uint32_t imm5 = get_bits(instruction, 20, 5);
      switch (funct1) {
        case 0x0:
          proc.addi(rd, rs, imm);
          break;
        case 0x1:
          if (funct2 == 0) {
            proc.slli(rd, rs, imm5);
          } else {
            throw std::invalid_argument("Invalid immediate funct3");
          }
          break;
        case 0x2:
          proc.slti(rd, rs, imm);
          break;
        case 0x3:
          proc.sltiu(rd, rs, imm);
          break;
        case 0x4:
          proc.xori(rd, rs, imm);
          break;
        case 0x5:
          if (funct2 == 0) {
            proc.srli(rd, rs, imm5);
          } else if (funct2 == 0x20) {
            proc.srai(rd, rs, imm5);
          } else {
            throw std::invalid_argument("Invalid immediate funct3");
          }
          break;
        case 0x6:
          proc.ori(rd, rs, imm);
          break;
        case 0x7:
          proc.andi(rd, rs, imm);
          break;
        default:
          throw std::invalid_argument("Invalid immediate funct3");
      }
      break;
    }
    case Opcode::kAuipc: {
      const uint32_t rd = get_bits(instruction, 7, 5);
      const uint32_t imm = get_bits(instruction, 12, 20);
      proc.auipc(rd, imm);
      break;
    }
    case Opcode::kStore: {
      const uint32_t funct = get_bits(instruction, 12, 3);
      const uint32_t rs1 = get_bits(instruction, 15, 5);
      const uint32_t rs2 = get_bits(instruction, 20, 5);
      const uint32_t offset = store_offset(instruction);
      switch (funct) {
        case 0x0:
          proc.sb(rs2, offset, rs1);
          break;
        case 0x1:
          proc.sh(rs2, offset, rs1);
          break;
        case 0x2:
          proc.sw(rs2, offset, rs1);
          break;
        default:
          throw std::invalid_argument("Invalid store funct3");
      }
      break;
    }
    case Opcode::kAlu: {
      const uint32_t rd = get_bits(instruction, 7, 5);
      const uint32_t funct1 = get_bits(instruction, 12, 3);
      const uint32_t rs1 = get_bits(instruction, 15, 5);
      const uint32_t rs2 = get_bits(instruction, 20, 5);
      const uint32_t funct2 = get_bits(instruction, 25, 7);
      switch (funct1) {
        case 0x0:
          if (funct2 == 0x0) {
            proc.add(rd, rs1, rs2);
          } else if (funct2 == 0x1) {
            proc.mul(rd, rs1, rs2);
          } else {
            proc.sub(rd, rs1, rs2);
          }
          break;
        case 0x1:
          if (funct2 == 0x1) {
            proc.mulh(rd, rs1, rs2);
          } else {
            proc.sll(rd, rs1, rs2);
          }
          break;
        case 0x2:
          if (funct2 == 0x1) {
            proc.mulhsu(rd, rs1, rs2);
          } else {
            proc.slt(rd, rs1, rs2);
          }
          break;
        case 0x3:
          if (funct2 == 0x1) {
            proc.mulhu(rd, rs1, rs2);
          } else {
            proc.sltu(rd, rs1, rs2);
          }
          break;
        case 0x4:
          if (funct2 == 0x1) {
            proc.div(rd, rs1, rs2);
          } else {
            proc.xorr(rd, rs1, rs2);
          }
          break;
        case 0x5:
          if (funct2 == 0x0) {
            proc.srl(rd, rs1, rs2);
          } else if (funct2 == 0x1) {
            proc.divu(rd, rs1, rs2);
          } else {
            proc.sra(rd, rs1, rs2);
          }
          break;
        case 0x6:
          if (funct2 == 0x1) {
            proc.rem(rd, rs1, rs2);
          } else {
            proc.orr(rd, rs1, rs2);
          }
          break;
        case 0x7:
          if (funct2 == 0x1) {
            proc.remu(rd, rs1, rs2);
          } else {
            proc.andr(rd, rs1, rs2);
          }
          break;
        default:
          throw std::invalid_argument("Invalid alu funct3");
      }
      break;
    }
    case Opcode::kLui: {
      const uint32_t rd = get_bits(instruction, 7, 5);
      const uint32_t imm = get_bits(instruction, 12, 20);
      proc.lui(rd, imm);
      break;
    }
    case Opcode::kBranch: {
      const uint32_t rs1 = get_bits(instruction, 15, 5);
      const uint32_t rs2 = get_bits(instruction, 20, 5);
      const uint32_t funct = get_bits(instruction, 12, 3);
      const uint32_t offset = branch_offset(instruction);
      switch (funct) {
        case 0x0:
          proc.beq(rs1, rs2, offset);
          break;
        case 0x1:
          proc.bne(rs1, rs2, offset);
          break;
        case 0x4:
          proc.blt(rs1, rs2, offset);
          break;
        case 0x5:
          proc.bge(rs1, rs2, offset);
          break;
        case 0x6:
          proc.bltu(rs1, rs2, offset);
          break;
        case 0x7:
          proc.bgeu(rs1, rs2, offset);
          break;
        default:
          throw std::invalid_argument("Invalid branch funct3");
      }
      break;
    }
    case Opcode::kJalr: {
      const uint32_t funct = get_bits(instruction, 12, 3);
      const uint32_t rd = get_bits(instruction, 7, 5);
      const uint32_t rs = get_bits(instruction, 15, 5);
      const uint32_t offset = get_offset(instruction);
      if (funct)
        throw std::invalid_argument("Invalid jalr funct3");
      proc.jalr(rd, offset, rs);
      break;
    }
    case Opcode::kJal: {
      const uint32_t rd = get_bits(instruction, 7, 5);
      const uint32_t offset = jal_offset(instruction);
      proc.jal(rd, offset);
      break;
    }
    case Opcode::kSystem: {
      const uint32_t funct = get_bits(instruction, 12, 3);
      const uint32_t imm = get_bits(instruction, 20, 12);
      if (funct == 0x0 && imm == 0x0) {
        proc.ecall();
      } else if (funct == 0x0 && imm == 0x1) {
        proc.ebreak();
      } else {
        throw std::invalid_argument("Invalid alu funct3/funct7 combination");
      }
      break;
    }
    case Opcode::kFence: {
      proc.fence();
      break;
    }
    default:
      throw std::invalid_argument("Unknown opcode");
  }
}
