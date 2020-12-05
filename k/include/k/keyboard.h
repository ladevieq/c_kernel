#ifndef K_KEYBOARD_H
#define K_KEYBOARD_H

#include <k/types.h>

#define KEYBOARD_IO_BUFFER_PORT         0x60
#define KEYBOARD_STATUS_REGISTER_PORT   0x64

#define KEYBOARD_IO_QUEUE_SIZE          256

void pullkey(void);

s32 getkey(void);

#endif				/* !KEYBOARD_H_ */
