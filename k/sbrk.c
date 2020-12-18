#include <k/sbrk.h>

#include <stdio.h>

void* brk = NULL;

void set_brk(void* new_brk) {
    brk = new_brk;
}

void* sbrk(ssize_t increment) {
    printf("Old brk : %u\n", brk);
    printf("New brk : %u\n", brk + increment);
    brk += increment;
    return brk;
}
