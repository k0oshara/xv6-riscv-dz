#include "types.h"
#include "memlayout.h"
#include "rtc.h"

uint32 rtc_read_low() {
  return *(volatile uint32*)RTC_TIME_LOW;
}

uint32 rtc_read_high() {
  return *(volatile uint32*)RTC_TIME_HIGH;
}

uint64 rtc_get_time() {
  uint32 low = rtc_read_low();
  uint32 high = rtc_read_high();
  return ((uint64)high << 32) | low;
}
