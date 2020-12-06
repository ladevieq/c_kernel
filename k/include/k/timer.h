#ifndef K_TIMER_H
#define K_TIMER_H

#include <k/types.h>

#define PIT_COUNTER_0          0x40
#define PIT_COUNTER_1          0x41
#define PIT_COUNTER_2          0x42
#define PIT_CONTROL_REGISTER   0x43

#define PIT_INTERNAL_FREQUENCY      1193182U
#define PIT_DESIRED_FREQUENCY       100U

void init_TIMER(void);

void tick(void);

u64 gettick(void);

#endif				/* !TIMER_H_ */
