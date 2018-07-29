#include <stdio.h>
#include <assert.h>

#include "file.h"
#include "inode.h"
#include "diskimg.h"

int file_getblock(struct unixfilesystem *fs, int inumber, int blockNum, void *buf) {
  struct inode in;
  if (inode_iget(fs, inumber, &in) < 0) {
    fprintf(stderr, "Can't read inode %d \n", inumber);
    return -1;
  }
  int actualBlockNum = inode_indexlookup(fs, &in, blockNum);
  if (actualBlockNum < 0) {
    fprintf(stderr, "Can't get block num %d for inode %d\n", blockNum, inumber);
    return -1;
  }
  return diskimg_readsector(fs->dfd, actualBlockNum, buf);
}
