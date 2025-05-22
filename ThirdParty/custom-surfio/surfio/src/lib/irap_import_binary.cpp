#include "include/irap.h"
#include "include/irap_import.h"
#include "mmap_wrapper/mmap_wrapper.h"
#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <filesystem>
#include <format>
#include <ranges>
#include <span>
#include <stdexcept>

namespace fs = std::filesystem;

namespace surfio::irap {
template <typename T> T swap_byte_order(const T& value) {
  auto tmp = std::bit_cast<std::array<std::byte, sizeof(T)>>(value);
  std::ranges::reverse(tmp);
  T result = std::bit_cast<T>(tmp);
  return result;
}

template <IsLittleEndianNumeric T>
const char* read_32bit_value(const char* begin, const char* end, T& value) {
  if (begin + 4 > end)
    throw std::length_error("End of file reached unexpectedly");

  if constexpr (std::is_integral_v<T>)
    value = swap_byte_order(reinterpret_cast<const int32_t&>(*begin));
  else
    value = swap_byte_order(reinterpret_cast<const float&>(*begin));

  return begin + 4;
}

template <IsLittleEndianNumeric T>
const char*
read_and_check_value(const char* begin, const char* end, const T& check, std::string_view err_msg) {
  T value;
  auto ptr = read_32bit_value(begin, end, value);
  if (value != check)
    throw std::domain_error(std::string(err_msg));

  return ptr;
}

template <IsLittleEndianNumeric... T>
const char* read_chunk(const char* begin, const char* end, T&... values) {
  constexpr int32_t chunk_size = sizeof...(values) * 4;
  auto ptr = begin;

  ptr = read_and_check_value(ptr, end, chunk_size, "Incorrect chunk size");
  ([&] { ptr = read_32bit_value(ptr, end, values); }(), ...);
  ptr = read_and_check_value(ptr, end, chunk_size, "Chunk size mismatch");

  return ptr;
}

std::tuple<irap_header, const char*> get_header_binary(std::span<const char> buffer) {
  // header is 100 bytes. 6 chunk guards (24 bytes), 19 values (76 bytes)
  if (buffer.size() < 100)
    throw std::length_error("Header must be at least 100 bytes long");
  irap_header header;
  int32_t dummy;
  float fdummy;
  auto ptr = &*buffer.begin();
  auto end = &*buffer.end();
  try {
    ptr = read_chunk(
        ptr, end, dummy, header.nrow, header.xori, header.xmax, header.yori, header.ymax,
        header.xinc, header.yinc
    );
    if (dummy != irap_header::id)
      std::domain_error(
          std::format("Incorrect magic number: {}. Expected {}", dummy, irap_header::id)
      );
    ptr = read_chunk(ptr, end, header.ncol, header.rot, header.xrot, header.yrot);
    ptr = read_chunk(ptr, end, fdummy, fdummy, dummy, dummy, dummy, dummy, dummy);
  } catch (const std::exception& e) {
    throw std::domain_error(std::format("Failed to read irap headers: {}", e.what()));
  }

  return {header, ptr};
}

std::vector<float> get_values_binary(const char* start, const char* end, int ncol, int nrow) {
  const size_t nvalues = ncol * nrow;
  auto values = std::vector<float>(nvalues);
  auto ptr = start;

  // chunk guards tell you how many bytes there are to read in a chunk.
  // there is a matching guard at the end of each chunk.
  int32_t chunk_size;
  for (auto i = 0u; i < nvalues;) {
    ptr = read_32bit_value(ptr, end, chunk_size);
    size_t values_left = chunk_size / 4; // each value is 32 bit
    for (auto j = 0u; j < values_left; ++j, ++i) {
      float value;
      ptr = read_32bit_value(ptr, end, value);
      auto ic = column_major_to_row_major_index(i, ncol, nrow);
      values[ic] = value;
    }
    ptr = read_and_check_value(ptr, end, chunk_size, "Block size mismatch");
  }

  return values;
}

irap from_binary_file(const fs::path& file) {
  auto buffer = mmap::mmap_file(file);
  auto [header, ptr] = get_header_binary(buffer);
  auto values = get_values_binary(ptr, buffer.end(), header.ncol, header.nrow);

  return {.header = header, .values = std::move(values)};
}

irap from_binary_buffer(std::span<const char> buffer) {
  auto buffer_end = &*buffer.begin() + buffer.size();
  auto [header, ptr] = get_header_binary(buffer);
  auto values = get_values_binary(ptr, buffer_end, header.ncol, header.nrow);

  return {.header = header, .values = std::move(values)};
}
} // namespace surfio::irap
