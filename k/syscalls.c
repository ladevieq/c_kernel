#include <k/syscalls.h>
#include <k/kstd.h>
#include <k/iso9660.h>
#include <k/keyboard.h>
#include <k/timer.h>

typedef void (*syscall_wrapper)(u32, u32, u32);

static syscall_wrapper syscall_dispatch_table[NR_SYSCALL] = { NULL };

void empty_wrapper() {}

// Define syscalls wrappers
#define X(name, func) void wrapper_##name(u32 ebx, u32 ecx, u32 edx) { \
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
void dispatch_syscall(u32 syscall_index, u32 ebx, u32 ecx, u32 edx) {
    syscall_dispatch_table[syscall_index](ebx, ecx, edx);
}
