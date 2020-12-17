#include <k/syscalls.h>
#include <k/kstd.h>
#include <k/iso9660.h>
#include <k/keyboard.h>
#include <k/timer.h>

typedef u32 (*syscall_wrapper)(u32, u32, u32);

static syscall_wrapper syscall_dispatch_table[NR_SYSCALL] = { NULL };

u32 empty_wrapper() {
    return 0;
}

// Define syscalls wrappers
#define X(name, func) u32 wrapper_##name(u32 ebx, u32 ecx, u32 edx) { \
    return func; \
}
#include <k/syscalls.def>
#undef X

// Init dispatch table
void init_syscalls_dispatch_table(void) {
#define X(name, func) syscall_dispatch_table[SYSCALL_##name - 1] = &wrapper_##name;
#include <k/syscalls.def>
#undef X
}

// EAX contains the syscall index for the dispatch table
u32 dispatch_syscall(u32 syscall_index, u32 ebx, u32 ecx, u32 edx) {
    return syscall_dispatch_table[syscall_index](ebx, ecx, edx);
}
