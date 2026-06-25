#pragma once

constexpr uint32_t kMemorySize = 256 * 1024; 
constexpr uint32_t kCacheSize = 4096;
constexpr uint32_t kCacheLineSize = 32;
constexpr uint32_t kCacheLineCount = 128;
constexpr uint32_t kCacheSetCount = 32;
constexpr uint32_t kCacheWay = 4;
constexpr uint32_t kCacheIndexLen = 5;
constexpr uint32_t kCacheOffsetLen = 5;
constexpr uint32_t kAddressLen = 18;
constexpr uint32_t kCacheTagLen = 8;