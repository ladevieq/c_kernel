#ifndef IO_H_
#define IO_H_

#include <k/types.h>

#define COM1 0x3f8
#define DIVISOR_LATCH_OFFSET 0x0
#define IER_OFFSET 0x1
#define FCR_OFFSET 0x2
#define LCR_OFFSET 0x3

#define FIFO_SIZE 0xf

static inline void outb(u16 port, u8 val)
{
	asm volatile ("outb %0, %1" : /* No output */ : "a"(val), "d"(port));
}

static inline u8 inb(u16 port)
{
	u8 res;

	asm volatile ("inb %1, %0" : "=&a"(res) : "d"(port));

	return res;
}

static inline void outw(u16 port, u16 val)
{
	asm volatile ("outw %0, %1" : /* No output */ : "a"(val), "d"(port));
}

static inline u16 inw(u16 port)
{
	u16 res;

	asm volatile ("inw %1, %0" : "=&a"(res) : "d"(port));

	return res;
}

struct InterruptEnableRegister {
	union {
		struct {
			u8 enable_received_data_available_interrupt: 1;
			u8 enable_transmitter_holding_register_empty_interrupt: 1;
			u8 enable_receiver_line_status_interrupt: 1;
			u8 enable_modem_status_interrupt: 1;
			u8 enable_sleep_mode: 1;
			u8 enable_low_power_mode: 1;
			u8 reserved: 2; // Always 0
		};
		u8 data;
	};
};

struct FifoControlRegister {
	union {
		struct {
			u8 enable_fifos: 1;
			u8 clear_receive_fifo: 1;
			u8 clear_transmit_fifo: 1;
			u8 dma_mode_select: 1;
			u8 reserved: 1; // Always 0
			u8 enable_64bytes_fifo: 1;
			u8 trigger_threshlold_value: 2;
		};
		u8 data;
	};
};

enum DATA_WORDS_PARITY_MODE {
	DATA_WORDS_PARITY_NO_PARITY,
	DATA_WORDS_PARITY_ODD_PARITY,
	DATA_WORDS_PARITY_EVEN_PARITY = 3,
	DATA_WORDS_PARITY_MARK_PARITY = 5,
	DATA_WORDS_PARITY_SPACE_PARITY = 7,
};

enum DATA_WORDS_SIZES {
	DATA_WORDS_SIZE_5_BIT,
	DATA_WORDS_SIZE_6_BIT,
	DATA_WORDS_SIZE_7_BIT,
	DATA_WORDS_SIZE_8_BIT,
};

struct LineControlRegister {
	union {
		struct {
			u8 data_word_size: 2;
			u8 stop_bits: 1;
			u8 parity_mode: 3;
			u8 break_enabled: 1;
			u8 divisor_latch_access_bit: 1;
		};
		u8 data;
	};
};

struct LineStatusRegister {
	union {
		struct {
			u8 data_ready: 1;
			u8 overrun_error: 1;
			u8 parity_error: 1;
			u8 framing_error: 1;
			u8 break_interrupt: 1;
			u8 empty_transmitter_register: 1;
			u8 empty_data_register: 1;
			u8 error_in_fifo: 1;
		};
		u8 data;
	};
};

void init_COM1();

size_t write(const char* buf, size_t count);


#endif				/* !IO_H_ */
