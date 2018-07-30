
#include "pathname.h"
#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define PATH_SEP "/"

int pathname_lookup(struct unixfilesystem *fs, const char *pathname) {
  if (strcmp(pathname, PATH_SEP) == 0) return ROOT_INUMBER;
  struct direntv6 dirEnt;
  char pathname_cpy[strlen(pathname)+1];
  strcpy(pathname_cpy, pathname);
  char *tok = strtok(pathname_cpy + 1, PATH_SEP);
  return pathname_lookup_helper(fs, ROOT_INUMBER, tok, &dirEnt);
}

int pathname_lookup_helper(struct unixfilesystem *fs, const int dirinumber,
                           char *tok, struct direntv6 *dirEnt) {
  if (directory_findname(fs, tok, dirinumber, dirEnt) < 0) {
    return -1;
  }
  tok = strtok(NULL, PATH_SEP);
  return (tok == NULL)
         ? dirEnt->d_inumber
         : pathname_lookup_helper(fs, dirEnt->d_inumber, tok, dirEnt);
}
