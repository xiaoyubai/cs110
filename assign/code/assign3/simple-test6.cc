#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string>

using namespace std;

int main(int argc, char *argv[]) {
  struct stat stats;
  const string fileName = "simple-test6.cc";
  stat(fileName.c_str(), &stats);
  int fd = open(fileName.c_str(), O_RDONLY);
  mmap(0, stats.st_size, PROT_READ, MAP_SHARED, fd, 0);
  return 0;
}
