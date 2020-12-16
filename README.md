
# c_kernel
A project used to learn how operating systems work under the hood

## Capabilities

### Interrupts
Interrupts are fully working. An IDT is setup and it only misses handler for every interrupts.
This will change in the future as i implement more and more features into the kernel.

Interrupts are handled by assembly routines (ISRs). Those ISRs pushes vector index and error onto the stack and then calls the C interrupt wrapper. The wrapper will then dispatch the interrupt handling to the correct C function.

### PIC
The PIC is configured to allow only keyboard and timer interrupts as i don't need more that those two.
Later on other interrupts will be forwarded to the CPU by the PIC as i need them.

### Memory
For now memory use segmentation.

All segments are mapped to the whole available memory. It is planned to splits segments once useland programs become availables and then to switch to paging.

### Userland
Userland is not yet implemented.

### Atapi
The kernel includes an atapi driver that allow reading block of memory from a CD-ROM.

As of today i don't plan to implement write to devices using ATAPI.

### Filesystem
The filesystem used by the kernel is ISO as we only read from CD-ROMs through the ATAPI driver.
The filesystem API for ISO filesystem includes function to locate a file / dir.
The API will be extended as i need more functions to work with ISO.

Once again i plan to use another filesystem later but for now it's enough.

### Syscalls
Some syscalls are already implemented like IO syscalls, gettick, etc.
But they are not yet available from userland, this will change once userland is setup and can run executables.

### Executables
The kernel is already capable of loading and running ELF executables.
Some kernel features are currently missing so nothing happen when running an executable.


## Status
Currently syscalls are being setup in the kernel for the ELF executable to really do something
