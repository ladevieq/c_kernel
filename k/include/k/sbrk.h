#ifndef K_SBRK_H
#define K_SBRK_H

#include <k/kstd.h>

void set_brk(void* new_brk);

void* sbrk(ssize_t increment);

#endif				/* !SBRK_H_ */
