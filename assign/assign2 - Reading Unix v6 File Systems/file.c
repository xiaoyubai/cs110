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
  int size = inode_getsize(&in);
  if (size <= 0) {
    fprintf(stderr, "File with inode %d has size <= 0\n", inumber);
    return -1;
  }
  int numBlocks = (size + DISKIMG_SECTOR_SIZE - 1) / DISKIMG_SECTOR_SIZE;
  int maxBlockNum = numBlocks-1;
  int actualBlockNum = inode_indexlookup(fs, &in, blockNum);
  if (actualBlockNum < 0) {
    fprintf(stderr, "Can't get block num %d for inode %d\n", blockNum, inumber);
    return -1;
  }
  int bytesRead = diskimg_readsector(fs->dfd, actualBlockNum, buf);
  if (bytesRead < 0) {
    fprintf(stderr, "There was a problem reading from the block %d\n", blockNum);
    return -1;
  }
  int validBytes = (blockNum < maxBlockNum)
         ? DISKIMG_SECTOR_SIZE
         : size - maxBlockNum * DISKIMG_SECTOR_SIZE;
  return validBytes;
}
