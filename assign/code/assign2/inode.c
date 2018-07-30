#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "inode.h"
#include "diskimg.h"

#define INODE_SIZE 32  // size in bytes
#define INODE_PER_BLOCK (DISKIMG_SECTOR_SIZE/INODE_SIZE)
#define BLOCKS_PER_INDIR 256
#define NUM_INDIR_BLOCKS 7
#define TOTAL_BLOCKS_FROM_INDIR (BLOCKS_PER_INDIR * NUM_INDIR_BLOCKS)

int inode_iget(struct unixfilesystem *fs, int inumber, struct inode *inp) {
  int offset = (inumber-1) / INODE_PER_BLOCK;
  struct inode buf[INODE_PER_BLOCK];
  int bytesRead = diskimg_readsector(fs->dfd, INODE_START_SECTOR + offset, buf);
  if (bytesRead != DISKIMG_SECTOR_SIZE) {
    fprintf(stderr, "Error reading inode %d\n", inumber);
    return -1;
  }
  *inp = buf[(inumber-1) % INODE_PER_BLOCK];
  return 0;
}

int inode_indexlookup(struct unixfilesystem *fs, struct inode *inp, int blockNum) {
  if (!(inp->i_mode & ILARG)) {
    // not a large file
    return inp->i_addr[blockNum];
  }
  uint16_t buf[BLOCKS_PER_INDIR];
  if (blockNum < TOTAL_BLOCKS_FROM_INDIR) {
    // singly indirect
    // each block 256 * 2-byte block numbers
    int indirBlockNum = blockNum / BLOCKS_PER_INDIR;
    diskimg_readsector(fs->dfd, inp->i_addr[indirBlockNum], buf);
    return buf[blockNum % BLOCKS_PER_INDIR];
  } else {
    // doubly indirect
    int indirBlockNum = (blockNum - TOTAL_BLOCKS_FROM_INDIR) / BLOCKS_PER_INDIR;
    diskimg_readsector(fs->dfd, inp->i_addr[7], buf);
    diskimg_readsector(fs->dfd, buf[indirBlockNum], buf);
    return buf[blockNum % BLOCKS_PER_INDIR];
  }
}

int inode_getsize(struct inode *inp) {
  return ((inp->i_size0 << 16) | inp->i_size1); 
}
