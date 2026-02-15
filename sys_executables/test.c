
void _start(void) {
    for (;;) {
        __asm__ volatile (
            "movl $1, %%eax\n"
            "int $0x80\n"
            :
            :
            : "eax"
        );
    }
}