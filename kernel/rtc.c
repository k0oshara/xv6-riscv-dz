#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "memlayout.h"
#include "rtc.h"
#include "spinlock.h"

struct spinlock rtclock;

void rtc_init(void) {
  initlock(&rtclock, "rtc");
}

uint32 rtc_read_low() {
  return *(volatile uint32*)RTC_TIME_LOW;
}

uint32 rtc_read_high() {
  return *(volatile uint32*)RTC_TIME_HIGH;
}

uint64 rtc_get_time() {
  uint32 high1, high2, low;

  acquire(&rtclock);
  do {
    high1 = rtc_read_high();
    low = rtc_read_low(); 
    high2 = rtc_read_high();
  } while (high1 != high2);
  release(&rtclock);
  
  return ((uint64)high1 << 32) | low;
}
