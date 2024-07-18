#include <am.h>
#include <nemu.h>
#include <klib.h>


static uint64_t boot_time = 0;

static uint64_t read_time1() {
  uint64_t lo = inl(RTC_ADDR);
  uint64_t hi = inl(RTC_ADDR+4);
  uint64_t time1 = (hi << 32) | lo;
  return time1;
}

void __am_timer_init() {
  boot_time = read_time1();
}


void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  uptime->us = read_time1() - boot_time;
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}
