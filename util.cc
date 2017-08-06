#include "util.h"

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cassert>

std::string ReadFile(const std::string& filename) {
  int fh = open(filename.c_str(), O_RDONLY);
  assert(fh != -1);

  struct stat st;
  assert(fstat(fh, &st) == 0);
  
  std::string contents;
  contents.resize(static_cast<size_t>(st.st_size));

  assert(read(fh, &contents[0], static_cast<size_t>(st.st_size)) == st.st_size);
  assert(close(fh) == 0);

  return contents;
}

void WriteFile(const std::string& filename, const std::string& contents) {
  int fh = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
  assert(fh != -1);
  assert(write(fh, &contents[0], contents.size()) == static_cast<ssize_t>(contents.size()));
  assert(close(fh) == 0);
}
