#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

int main(int argc, char ** argv) {
  struct stat stat_buffer;
  return S_ISREG(stat_buffer.st_mode);
}
