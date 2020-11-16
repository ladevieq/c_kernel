#include <k/io.h>

void init_COM1() {
	struct InterruptEnableRegister ier;
	ier.enable_received_data_available_interrupt = 0;
	ier.enable_transmitter_holding_register_empty_interrupt = 1;
	ier.enable_receiver_line_status_interrupt = 0;
	ier.enable_modem_status_interrupt = 0;
	ier.enable_sleep_mode = 0;
	ier.enable_low_power_mode = 0;
	ier.reserved = 0;

	struct FifoControlRegister fcr;
	fcr.enable_fifos = 1;
	fcr.clear_receive_fifo = 1;
	fcr.clear_transmit_fifo = 1;
	fcr.dma_mode_select = 0;
	fcr.reserved = 0;
	fcr.enable_64bytes_fifo = 0;
	fcr.trigger_threshlold_value = 0b11;

	struct LineControlRegister lcr;
	lcr.data_word_size = DATA_WORDS_SIZE_8_BIT;
	lcr.stop_bits = 0;
	lcr.parity_mode = DATA_WORDS_PARITY_NO_PARITY;
	lcr.break_enabled = 0;
	lcr.divisor_latch_access_bit = 1; // Allow us to set the baud rate of the chip

	outb(COM1 + LCR_OFFSET, lcr.data);

	// Set the baud rate to 38400 bps
	outb(COM1 + DIVISOR_LATCH_OFFSET + 1, 0x00);
	outb(COM1 + DIVISOR_LATCH_OFFSET, 0x03);

	lcr.divisor_latch_access_bit = 0; // Prevent us from setting the baud rate of the chip

	outb(COM1 + LCR_OFFSET, lcr.data);
	outb(COM1 + IER_OFFSET, ier.data);
	outb(COM1 + FCR_OFFSET, fcr.data);
}

void wait_for_transmission() {
	struct LineStatusRegister lsr = { .data = inb(COM1 + 5) };
	do {
		lsr.data = inb(COM1 + 5);
	} while(lsr.empty_data_register != 1);
}

size_t write(const char* buf, size_t count) {
	for (size_t i = 0; i < count; i++) {
		if (i % FIFO_SIZE == 0) {
			wait_for_transmission();
		}

		if (buf[i] == '\n') {
			outb(COM1, '\r');
		}
		outb(COM1, buf[i]);
	}

	return count;
}
