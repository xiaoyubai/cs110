#ifndef _PATHNAME_H_
#define _PATHNAME_H_

#include "unixfilesystem.h"

/**
 * Returns the inode number associated with the specified pathname.  This need only
 * handle absolute paths.  Returns a negative number (-1 is fine) if an error is 
 * encountered.
 */
int pathname_lookup(struct unixfilesystem *fs, const char *pathname);

int pathname_lookup_helper(struct unixfilesystem *fs, const int dirinumber,
                           char *tok, struct direntv6 *dirEnt);

#endif // _PATHNAME_H_
