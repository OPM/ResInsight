#include "include/irap_import.h"
#include "mmap_wrapper/mmap_wrapper.h"
#include <algorithm>
#include <charconv>
#include <filesystem>
#include <format>

namespace fs = std::filesystem;

namespace surfio::irap {
auto locale = std::locale("C");
auto& facet = std::use_facet<std::ctype<char>>(locale);
auto is_space = [](char ch) { return facet.is(std::ctype_base::space, ch); };

template <typename T, typename... U>
const char* read_headers(const char* start, const char* end, T& arg, U&... args) {
  start = std::find_if_not(start, end, is_space);
  auto [ptr, ec] = std::from_chars(start, end, arg);
  if (ec != std::errc{})
    throw std::domain_error("Failed to read irap headers");

  if constexpr (sizeof...(args) > 0)
    ptr = read_headers(ptr, end, args...);

  return ptr;
}

std::tuple<irap_header, const char*> get_header(const char* start, const char* end) {
  int magic_header;
  int idum;
  irap_header head;
  auto ptr = read_headers(
      start, end, magic_header, head.nrow, head.xinc, head.yinc, head.xori, head.xmax, head.yori,
      head.ymax, head.ncol, head.rot, head.xrot, head.yrot, idum, idum, idum, idum, idum, idum, idum
  );
  if (magic_header != irap_header::id)
    throw std::runtime_error(
        std::format(
            "First value in irap ascii file is incorrect. "
            "Irap ASCII. Expected: {}, got: {}",
            irap_header::id, std::to_string(magic_header)
        )
    );

  if (head.rot < 0.0)
    head.rot += 360.0;

  if (head.ncol < 0 || head.nrow < 0)
    throw std::domain_error("Incorrect dimensions encountered while importing Irap ASCII");

  return {head, ptr};
}

std::vector<float> get_values(const char* start, const char* end, int ncol, int nrow) {
  const size_t nvalues = ncol * nrow;
  auto values = std::vector<float>(nvalues);
  for (auto i = 0u; i < nvalues; ++i) {
    float value;

    start = std::find_if_not(start, end, is_space);
    if (start == end)
      throw std::length_error(
          std::format(
              "End of file reached before reading all values. Expected: {}, "
              "got {}",
              nvalues, i
          )
      );

    auto result = std::from_chars(start, end, value);
    start = result.ptr;
    if (result.ec != std::errc())
      throw std::domain_error("Failed to read values during Irap ASCII import.");

    if (value == UNDEF_MAP_IRAP)
      value = std::numeric_limits<float>::quiet_NaN();

    auto ic = column_major_to_row_major_index(i, ncol, nrow);
    values[ic] = value;
  }

  return values;
}

irap from_ascii_file(const fs::path& file) {
  auto buffer = mmap::mmap_file(file);

  auto [head, ptr] = get_header(buffer.begin(), buffer.end());
  auto values = get_values(ptr, buffer.end(), head.ncol, head.nrow);

  return {.header = std::move(head), .values = std::move(values)};
}

irap from_ascii_string(std::string_view buffer) {
  auto buffer_begin = buffer.data();
  auto buffer_end = buffer_begin + buffer.size();
  auto [head, ptr] = get_header(buffer_begin, buffer_end);
  auto values = get_values(ptr, buffer_end, head.ncol, head.nrow);

  return {.header = std::move(head), .values = std::move(values)};
}
} // namespace surfio::irap
