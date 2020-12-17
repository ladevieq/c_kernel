.text

.global interrupt_handler
isr:
    pusha
    push %esp
    call interrupt_handler
    add $4, %esp
    popa
    // Remove error code and interrupt code from the stack
    add $8, %esp
    iret

.macro gen_isr int_code
.global isr_\int_code
.type isr_\int_code, @function
isr_\int_code:
    pushl $0
    pushl $\int_code
    jmp isr
.endm

.macro gen_error_code_isr int_code
.global isr_\int_code
.type isr_\int_code, @function
isr_\int_code:
    pushl $\int_code
    jmp isr
.endm

.altmacro

// Define exceptions (32 first interrupt codes) except reserved ones
.set i,0
.rept 8
    gen_isr %i
    .set i,i+1
.endr

gen_error_code_isr 8
gen_isr 9
gen_error_code_isr 10
gen_error_code_isr 11
gen_error_code_isr 12
gen_error_code_isr 13
gen_error_code_isr 14
gen_isr 16
gen_error_code_isr 17
gen_isr 18
gen_isr 19
gen_isr 20
gen_error_code_isr 21

.set i,32
.rept 224
    gen_isr %i
    .set i,i+1
.endr
