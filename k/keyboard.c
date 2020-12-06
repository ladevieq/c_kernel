#include <k/keyboard.h>
#include <k/io.h>

#include <stdio.h>

// key numbers queue 
static u8 KEYS_QUEUE[KEYBOARD_IO_QUEUE_SIZE] = {'\0'};
static size_t DEQUEUE_INDEX = 0;
static size_t ENQUEUE_INDEX = 0;

void enqueue_key(u8 key) {
    if (ENQUEUE_INDEX - DEQUEUE_INDEX < KEYBOARD_IO_QUEUE_SIZE) {
        KEYS_QUEUE[ENQUEUE_INDEX % KEYBOARD_IO_QUEUE_SIZE] = key;
        ENQUEUE_INDEX++;
    }
}

s32 dequeue_key() {
    if (DEQUEUE_INDEX < ENQUEUE_INDEX) {
        u8 key = KEYS_QUEUE[DEQUEUE_INDEX];
        DEQUEUE_INDEX++;

        if (DEQUEUE_INDEX == KEYBOARD_IO_QUEUE_SIZE) {
            DEQUEUE_INDEX -= KEYBOARD_IO_QUEUE_SIZE;
            ENQUEUE_INDEX -= KEYBOARD_IO_QUEUE_SIZE;
        }

        return (s32)key;
    }

    return -1;
}

void pullkey(void) {
    u8 scancode = inb(KEYBOARD_IO_BUFFER_PORT);

    if ((scancode >> 7) == 0) {
        enqueue_key(scancode);
    }
}

s32 getkey(void) {
    return dequeue_key();
}
