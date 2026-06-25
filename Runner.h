#pragma once

#include <cstdint>

#include "Proc.h"

void DoInstruction(Proc& proc, uint32_t instruction);

void Runner(Proc& proc, IMemory& memory);
