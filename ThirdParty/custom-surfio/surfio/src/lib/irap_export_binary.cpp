#include "include/irap.h"
#include "include/irap_export.h"
#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

namespace surfio::irap {
template <IsLittleEndianNumeric T> void write_32bit_binary_value(char*& bufptr, T&& value) {
  std::array<char, 4> tmp;
  if constexpr (std::is_integral_v<std::decay_t<T>>)
    tmp = std::bit_cast<std::array<char, 4>>(static_cast<int32_t>(value));
  else
    tmp = std::bit_cast<std::array<char, 4>>(static_cast<float>(value));
  std::ranges::reverse(tmp);
  std::memcpy(bufptr, &tmp[0], 4);
  bufptr += 4;
}

template <IsLittleEndianNumeric... T>
void write_32bit_binary_values(std::ostream& out, T&&... values) {
  constexpr size_t HEADER_SIZE = 100;
  char buf[HEADER_SIZE];
  char* bufptr = buf;
  (write_32bit_binary_value(bufptr, std::forward<T>(values)), ...);
  out.write(buf, HEADER_SIZE);
}

void write_header_binary(const irap_header& header, std::ostream& out) {
  write_32bit_binary_values(
      out, 32, irap_header::id, header.nrow, header.xori, header.xmax, header.yori, header.ymax,
      header.xinc, header.yinc, 32, 16, header.ncol, header.rot, header.xrot, header.yrot, 16, 28,
      0.f, 0.f, 0, 0, 0, 0, 0, 28
  );
}

void write_values_binary(surf_span values, std::ostream& out) {
  size_t written_on_line = 0;
  size_t chunk_length = 0;
  auto rows = values.extent(0);
  auto cols = values.extent(1);
  auto remaining = values.size();
  constexpr long CHUNK_SIZE = (PER_LINE_BINARY + 2) * sizeof(float);
  char buf[CHUNK_SIZE * 10];
  char* bufptr = buf;
  for (size_t j = 0; j < cols; j++) {
    for (size_t i = 0; i < rows; i++) {
      if (written_on_line == 0) {
        chunk_length = std::min(remaining, PER_LINE_BINARY);
        write_32bit_binary_value(bufptr, chunk_length * 4);
      }

      auto v = values[i, j];
      write_32bit_binary_value(bufptr, std::isnan(v) ? UNDEF_MAP_IRAP : v);

      if (++written_on_line == chunk_length) {
        write_32bit_binary_value(bufptr, chunk_length * 4);
        written_on_line = 0;
      }

      if (auto bytes_written = std::distance(buf, bufptr);
          sizeof(buf) - bytes_written < CHUNK_SIZE) {
        out.write(buf, bytes_written);
        bufptr = buf;
      }

      --remaining;
    }
  }
  if (auto dist = std::distance(buf, bufptr); dist)
    out.write(buf, dist);
}

void to_binary_file(const fs::path& file, const irap_header& header, surf_span values) {
  std::ofstream out(file, std::ios::binary);
  write_header_binary(header, out);
  write_values_binary(values, out);
}

void to_binary_file(const fs::path& file, const irap& data) {
  to_binary_file(
      file, data.header, surf_span{data.values.data(), data.header.ncol, data.header.nrow}
  );
}

std::string to_binary_buffer(const irap_header& header, surf_span values) {
  std::ostringstream out;
  write_header_binary(header, out);
  write_values_binary(values, out);
  return out.str();
}

std::string to_binary_buffer(const irap& data) {
  return to_binary_buffer(
      data.header, surf_span{data.values.data(), data.header.ncol, data.header.nrow}
  );
}
} // namespace surfio::irap
