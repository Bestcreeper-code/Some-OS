#ifndef TIME_H
#define TIME_H

#include <stdint.h>
#include <stdbool.h>
#include "kernel_data.h"
// #define TICKS_AMOUNT_POINTER 0x2700

typedef struct {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint16_t year;
} __attribute__((__packed__)) rtc_time_t;

extern uint64_t timer_ticks;

void pic_remap();
// PIT / Timer
void pit_init(void);
void timer_irq(void); // call this from irq_handler when IRQ0 fires

// Sleeping
void sleep(uint64_t ms);

uint64_t rdtsc();

bool rtc_read_time(rtc_time_t* time);

uint32_t rtc_to_unix_timestamp(rtc_time_t* rtc);
void rtc_add_seconds(rtc_time_t* t, uint32_t seconds);

#endif // TIME_H
