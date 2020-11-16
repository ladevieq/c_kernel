.text

.global interrupt_handler
isr:
    pusha
    call interrupt_handler
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

.altmacro
.set i,0
.rept 256
    gen_isr %i
    .set i,i+1
.endr
