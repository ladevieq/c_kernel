#ifndef SYSCALL_H_
#define SYSCALL_H_

#include <k/kstd.h>

void init_syscalls_dispatch_table(void);

u32 dispatch_syscall(u32 eax, u32 ebx, u32 ecx, u32 edx);

#endif				/* !SYSCALL_H_ */
