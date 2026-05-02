
#pragma once

#include <cstdint>
#include <span>
#include <vector>


std::string formatBytes(std::uint64_t bytes);
std::string formatTable(const std::span<std::vector<std::string>> rows);
