#include "time.h"
#include "Logger.h"
#include "asm.h"
#include "drivers.h"
#include "io.h"
#include "memory.h"
#include <stdint.h>

#define CMOS_ADDRESS 0x70
#define CMOS_DATA    0x71



#define PIT_COMMAND   0x43
#define PIT_CHANNEL0  0x40
#define PIT_FREQUENCY 1000  // 1000 Hz

uint64_t timer_ticks_ms;



// ----------------- PIC -----------------
int pic_remap() {
    Sys_log("remapping PIC...\n");
    // Initialize PICs in cascade mode
    outb(0x20, 0x11); // Start initialization (master PIC)
    outb(0xA0, 0x11); // Start initialization (slave PIC)
    
    outb(0x21, 0x20); // Remap master PIC vector offset to 0x20 (32)
    outb(0xA1, 0x28); // Remap slave PIC vector offset to 0x28 (40)
    
    outb(0x21, 0x04); // Tell master PIC there is a slave at IRQ2
    outb(0xA1, 0x02); // Tell slave PIC its cascade identity
    
    outb(0x21, 0x01); // Set 8086 mode for master PIC
    outb(0xA1, 0x01); // Set 8086 mode for slave PIC
    
    outb(0x21, 0x0);  // Clear master PIC mask (enable all IRQs)
    outb(0xA1, 0x0);  // Clear slave PIC mask (enable all IRQs)

    Sys_Success("PIC remapped successfully.\n");
}

// ----------------- PIT -----------------
REGISTER_DRIVER_CORE(pit, pit_init);
int pit_init() {
    Sys_log("initializing pit\n");
    force_alloc(TICKS_AMOUNT,sizeof(uint64_t));
    timer_ticks_ms=0;
    
    uint16_t divisor = 1193180 / PIT_FREQUENCY;
    
    outb(PIT_COMMAND, 0x36);                 // Channel 0, lobyte/hibyte, mode 3
    outb(PIT_CHANNEL0, divisor & 0xFF);      // Low byte
    outb(PIT_CHANNEL0, (divisor >> 8) & 0xFF); // High byte
    Sys_Success("PIT initialized.\n");
    return 0;
}



void timer_irq() {
    (timer_ticks_ms)++;
}

// ----------------- Sleep -----------------

void sleep(uint64_t ms) {
    
    uint64_t target = timer_ticks_ms+ ms;
    while (timer_ticks_ms< target)__asm__ volatile ("hlt");  
    
}

// ----------------- RDTSC -----------------

uint64_t rdtsc() {
    uint32_t hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}


// BCD to binary conversion
static uint8_t bcd_to_bin(uint8_t bcd) {
    return (bcd & 0x0F) + ((bcd >> 4) * 10);
}

static uint8_t read_rtc_register(uint8_t reg) {
    outb(CMOS_ADDRESS, reg);
    return inb(CMOS_DATA);
}

bool rtc_read_time(rtc_time_t* time) {
    if (!time) return false;

    // Wait for update to complete (bit 7 in status register A)
    outb(CMOS_ADDRESS, 0x0A);
    while (inb(CMOS_DATA) & 0x80);

    // Read values
    uint8_t sec  = read_rtc_register(0x00);
    uint8_t min  = read_rtc_register(0x02);
    uint8_t hour = read_rtc_register(0x04);
    uint8_t day  = read_rtc_register(0x07);
    uint8_t mon  = read_rtc_register(0x08);
    uint8_t year = read_rtc_register(0x09);
    uint8_t reg_b = read_rtc_register(0x0B);

    // If RTC is in BCD mode, convert values
    bool is_bcd = !(reg_b & 0x04);
    if (is_bcd) {
        sec  = bcd_to_bin(sec);
        min  = bcd_to_bin(min);
        hour = bcd_to_bin(hour);
        day  = bcd_to_bin(day);
        mon  = bcd_to_bin(mon);
        year = bcd_to_bin(year);
    }

    time->second = sec;
    time->minute = min;
    time->hour   = hour;
    time->day    = day;
    time->month  = mon;
    time->year   = 2000 + year; // assuming year is 00–99

    return true;
}


DWORD get_fattime (void)
{
    rtc_time_t time;
    rtc_read_time(&time);

    DWORD fattime = 0;
    fattime |= ((time.year - 1980) & 0x7F) << 25;  // Year since 1980
    fattime |= ((time.month) & 0x0F) << 21;     // Month (1-12)
    fattime |= (time.day & 0x1F) << 16;           // Day (1-31)
    fattime |= (time.hour & 0x1F) << 11;           // Hour (0-23)
    fattime |= (time.minute & 0x3F) << 5;             // Minute (0-59)
    fattime |= (time.second / 2) & 0x1F;              // Second (0-29)

    return fattime;
    
}

static const int days_in_month[] = {
    31, 28, 31, 30, 31, 30,
    31, 31, 30, 31, 30, 31
};

bool is_leap_year(int year) {
    return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
}

uint32_t rtc_to_unix_timestamp(rtc_time_t* rtc)
{
    uint32_t days = 0;

    for (int y = 1970; y < rtc->year; y++) {
        days += is_leap_year(y) ? 366 : 365;
    }

    for (int m = 1; m < rtc->month; m++) {
        days += days_in_month[m - 1];

        if (m == 2 && is_leap_year(rtc->year)) {
            days += 1;
        }
    }

    days += (rtc->day - 1);

    uint32_t seconds = (uint32_t)((uint64_t)days * 86400ULL);
    seconds += rtc->hour * 3600U;
    seconds += rtc->minute * 60U;
    seconds += rtc->second;

    return seconds;
}

void rtc_add_seconds(rtc_time_t* t, uint32_t seconds) {
    if (!t) return;

    // Add seconds and cascade
    t->second += seconds % 60;
    if (t->second >= 60) {
        t->second -= 60;
        t->minute++;
    }

    seconds /= 60;
    t->minute += seconds % 60;
    if (t->minute >= 60) {
        t->minute -= 60;
        t->hour++;
    }

    seconds /= 60;
    t->hour += seconds % 24;
    if (t->hour >= 24) {
        t->hour -= 24;
        t->day++;
    }

    // Add full days
    seconds /= 24;
    t->day += seconds;

    // Normalize day/month/year
    while (1) {
        int dim = days_in_month[t->month - 1];
        if (t->month == 2 && is_leap_year(t->year)) {
            dim = 29;
        }

        if (t->day <= dim) break;

        t->day -= dim;
        t->month++;

        if (t->month > 12) {
            t->month = 1;
            t->year++;
        }
    }
}

