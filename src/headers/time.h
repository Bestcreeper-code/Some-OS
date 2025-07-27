#ifndef TIME_H
#define TIME_H

#include <stdint.h>
#include <stdbool.h>

#define TICKS_AMOUNT_POINTER 0x2700

typedef struct {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint16_t year;
} rtc_time_t;


void pic_remap();
// PIT / Timer
void pit_init(void);
void timer_irq(void); // call this from irq_handler when IRQ0 fires

// Sleeping
void sleep(uint64_t ms);

uint64_t rdtsc();

bool rtc_read_time(rtc_time_t* time);



#endif // TIME_H
