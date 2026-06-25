#pragma once

#include <string>

#include "Proc.h"

void WriteFile(Proc& proc, IMemory& memory, const std::string& file_name, uint32_t start, uint32_t size);
