#include <k/timer.h>
#include <k/io.h>

static u64 TICK_COUNT = 0;

void init_TIMER(void) {
    // Initialize the first counter to mode 2
    u8 register_state = 0;
    register_state |= 0b00000100; // Set the mode
    register_state |= 0b00110000; // Set the read/write policy

    outb(register_state, PIT_CONTROL_REGISTER);

    outb(PIT_INTERNAL_FREQUENCY / PIT_DESIRED_FREQUENCY, PIT_COUNTER_0);
}

void tick(void) {
    TICK_COUNT++;
}

u64 gettick(void) {
    return TICK_COUNT * 100;
}
