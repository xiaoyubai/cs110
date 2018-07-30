#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include "file.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

int directory_findname(struct unixfilesystem *fs, const char *name,
		       int dirinumber, struct direntv6 *dirEnt) {
  struct inode in;
  int err = inode_iget(fs, dirinumber, &in);
  if (err < 0) return err;
  if (!(in.i_mode & IALLOC) || ((in.i_mode & IFMT) != IFDIR)) {
    /* Not allocated or not a directory */
    return -1;
  }
  int size = inode_getsize(&in);
  assert((size % sizeof(struct direntv6)) == 0);
  int numBlocks  = (size + DISKIMG_SECTOR_SIZE - 1) / DISKIMG_SECTOR_SIZE;
  char buf[DISKIMG_SECTOR_SIZE];
  struct direntv6 *dir = (struct direntv6 *) buf;
  for (int bno=0; bno<numBlocks; bno++) {
    int bytesLeft, numEntriesInBlock, i;
    bytesLeft = file_getblock(fs, dirinumber, bno, dir);
    if (bytesLeft < 0) {
      fprintf(stderr, "Error reading directory\n");
      return -1;
    }
    numEntriesInBlock = bytesLeft/sizeof(struct direntv6);
    for (i=0; i<numEntriesInBlock ; i++) {
      if (strcmp(name, dir[i].d_name) == 0) {
        *dirEnt = dir[i];
        return 0;
      }
    }
  }
  return -1;
}
