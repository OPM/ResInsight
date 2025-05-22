#pragma once

#include <bit>
#include <vector>

namespace surfio::irap {
constexpr float UNDEF_MAP_IRAP = 9999900.0000;

struct irap_header {
  static constexpr int id = -996;
  int ncol;
  int nrow;
  double xori = 0.;
  double yori = 0.;
  double xmax = 0.;
  double ymax = 0.;
  double xinc = 1.;
  double yinc = 1.;
  double rot = 0.;
  double xrot = 0.;
  double yrot = 0.;
  // We do not know what these values signify.
  // They are all 0 in files we have access to.
  // float unknown[2];
  // int more_unknown[5];
  friend bool operator==(irap_header, irap_header) = default;
  friend bool operator!=(irap_header, irap_header) = default;
};

struct irap {
  irap_header header;
  std::vector<float> values;
};

template <typename T>
concept IsLittleEndian = std::endian::native == std::endian::little;
template <typename T>
concept IsLittleEndianNumeric =
    (std::integral<std::decay_t<T>> || std::floating_point<std::decay_t<T>>) && IsLittleEndian<T>;
} // namespace surfio::irap
