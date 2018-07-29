#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "inode.h"
#include "diskimg.h"

#define INODE_SIZE 32  // size in bytes
#define INODE_PER_BLOCK (DISKIMG_SECTOR_SIZE/INODE_SIZE)
#define INODES_PER_INDIR 256
#define NUM_INDIR_BLOCKS 7
#define NUM_INDIR_INODES (INODES_PER_INDIR * NUM_INDIR_BLOCKS)

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

  if (blockNum < NUM_INDIR_INODES) {
    // singly indirect
    // each block 256 * 2-byte block numbers
    return inode_indexlookup_indirect(fs,inp, blockNum);
  } else {
    // doubly indirect
    blockNum = blockNum - NUM_INDIR_INODES;
    return inode_indexlookup_indirect(fs,inp, blockNum);
  }
}

int inode_indexlookup_indirect(struct unixfilesystem *fs, struct inode *inp, int blockNum) {
  int indirBlockNum = blockNum / INODES_PER_INDIR;
  uint16_t buf[INODES_PER_INDIR];
  diskimg_readsector(fs->dfd, indirBlockNum, buf);
  return buf[blockNum % INODES_PER_INDIR];
}

int inode_getsize(struct inode *inp) {
  return ((inp->i_size0 << 16) | inp->i_size1); 
}
