#ifndef K_FS_H
#define K_FS_H

#include <k/kstd.h>

int open(const char *pathname, int flags);

ssize_t read(int fd, void *buf, size_t count);

off_t seek(int fd, off_t offset, int whence);

int close(int fd);

#endif				/* !FS_H_ */
