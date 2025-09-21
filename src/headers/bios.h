#ifndef DEBUG
#define DEBUG

#define Setup_Base 0x600

#define REAL_MODE_ARGS_BASE ((void*)0x1000)
#define REAL_MODE_OUTPUT_BASE ((void*)0x1010)

void Realmode_run(void (*func)(void));

#endif // !DEBUG
