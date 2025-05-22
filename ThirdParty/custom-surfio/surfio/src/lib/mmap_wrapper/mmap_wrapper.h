#include <filesystem>
#include <memory>

namespace surfio::mmap {
struct internals;

class mmap_file {
public:
  mmap_file(const std::filesystem::path& file);
  ~mmap_file();
  const char* begin() const;
  const char* end() const;

private:
  std::unique_ptr<internals> d;
};
} // namespace surfio::mmap
