#pragma once

#include "irap.h"
#include <filesystem>

#if __cpp_lib_mdspan
#include <mdspan>
#else
#include <experimental/mdspan>
#endif

namespace surfio::irap {
#if __cpp_lib_mdspan
using std::dynamic_extent;
using std::extents;
using std::mdspan;
#else
using std::experimental::dynamic_extent;
using std::experimental::extents;
using std::experimental::mdspan;
#endif

constexpr size_t MAX_PER_LINE = 9; // Maximum accepted by some software
// write 8 values per block. this could make
// it easier to use simd to import the values
constexpr size_t PER_LINE_BINARY = 8;

using surf_span = mdspan<const float, extents<size_t, dynamic_extent, dynamic_extent>>;

void to_ascii_file(const std::filesystem::path& file, const irap_header& header, surf_span values);
void to_ascii_file(const std::filesystem::path& file, const irap& data);

std::string to_ascii_string(const irap_header& header, surf_span values);
std::string to_ascii_string(const irap& data);

void to_binary_file(const std::filesystem::path& file, const irap_header& header, surf_span values);
void to_binary_file(const std::filesystem::path& file, const irap& data);

std::string to_binary_buffer(const irap_header& header, surf_span values);
std::string to_binary_buffer(const irap& data);
} // namespace surfio::irap
