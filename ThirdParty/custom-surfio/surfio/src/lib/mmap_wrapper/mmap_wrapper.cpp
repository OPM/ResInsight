#include "mmap_wrapper.h"
#include "mio.hpp"
#include <format>
#include <stdexcept>

namespace fs = std::filesystem;

namespace surfio::mmap {
struct internals {
  mio::mmap_source handle;
};

mmap_file::mmap_file(const fs::path& file) {
  std::error_code ec;
  auto handle = mio::make_mmap_source(file.string(), ec);
  if (ec)
    throw std::runtime_error(
        std::format("failed to map file :{}, with error: {}", file.string(), ec.message())
    );

  d = std::make_unique<internals>(std::move(handle));
}

mmap_file::~mmap_file() {}
const char* mmap_file::begin() const { return d->handle.begin(); }
const char* mmap_file::end() const { return d->handle.end(); }
} // namespace surfio::mmap
