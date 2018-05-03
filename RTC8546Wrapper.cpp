#include <Arduino.h>
#include <time.h>
#include <Wire.h>
#include "RTC8546Wrapper.h"

#define REG_ADDR_CONTROL1      0x00
#define REG_ADDR_CONTROL2      0x01
#define REG_ADDR_SECONDS       0x02
#define REG_ADDR_MINUTES       0x03
#define REG_ADDR_HOURS         0x04
#define REG_ADDR_DAYS          0x05
#define REG_ADDR_WEEKDAYS      0x06
#define REG_ADDR_MONTHS        0x07
#define REG_ADDR_YEARS         0x08
#define REG_ADDR_MINUTE_ALARM  0x09
#define REG_ADDR_HOUR_ALARM    0x0A
#define REG_ADDR_DAY_ALARM     0x0B
#define REG_ADDR_WEEKDAY_ALARM 0x0C
#define REG_ADDR_CLOCKOUT_FREQ 0x0D
#define REG_ADDR_TIMER_CONTROL 0x0E
#define REG_ADDR_TIMER         0x0F

#define STOP_BIT                5     // CONTROL1
#define INTERRUPT_PERIODIC      4     // CONTROL2
#define ALARM_FLAG              3     // CONTROL2
#define TIMER_FLAG              2     // CONTROL2
#define ALARM_INTERRUPT_ENABLE  1     // CONTROL2
#define TIMER_INTERRUPT_ENABLE  0     // CONTROL2
#define VOLTAGE_LOW             7     // SECONDS
#define ALARM_ENABLE            7     // MIN ALARAM - WEEKDAY ALARAM
#define CLOCK_OUT_ENABLE        7     // CLKOUT
#define CLOCK_OUT_FREQ_32768Hz  0x00  // CLKOUT
#define CLOCK_OUT_FREQ_1024Hz   0x01  // CLKOUT
#define CLOCK_OUT_FREQ_32Hz     0x02  // CLKOUT
#define CLOCK_OUT_FREQ_1Hz      0x03  // CLKOUT
#define TIMER_ENABLE            7     // TIMER CONTROL
#define TIMER_CLOCK_4096Hz      0     // TIMER CONTROL
#define TIMER_CLOCK_64Hz        1     // TIMER CONTROL
#define TIMER_CLOCK_1Hz         2     // TIMER CONTROL
#define TIMER_CLOCK_1_60Hz      3     // TIMER CONTROL

#define MINUTES_MASK   0b01111111
#define HOURS_MASK     0b00111111
#define DAYS_MASK      0b00111111
#define WEEKDAYS_MASK  0b00000111
#define MONTHS_MASK    0b00011111

#define RTC8564_ADDR  0x51  // I2C 7bit address


// no name space functions
namespace {
  void rtc8564_write(uint8_t addr, uint8_t data) {
    Wire.beginTransmission(RTC8564_ADDR);
    Wire.write(addr);
    Wire.write(data);
    Wire.endTransmission(); 
  }
  
  uint8_t rtc8564_read(const uint8_t &addr) {
    Wire.beginTransmission(RTC8564_ADDR);
    Wire.write(addr);
    Wire.endTransmission();
    Wire.requestFrom(RTC8564_ADDR, 1);
    
    return Wire.read();
  }
  
  boolean rtc8564_test_bit(uint8_t addr, uint8_t bit_position) {
    uint8_t data;
  
    data = rtc8564_read(addr);
    data &= (0x01 << bit_position);
    if( data == 0x00 ) {
      return false;
    } else {
      return true;
    }
  }
  
  void rtc8564_set_bit(uint8_t addr, uint8_t bit_position) {
    uint8_t data;
  
    data = rtc8564_read( addr );
    data |= (0x01 << bit_position);
    rtc8564_write( addr, data ); 
  }

  void rtc8564_clear_bit(uint8_t addr, uint8_t bit_position) {
    uint8_t data;
  
    data = rtc8564_read( addr );
    data &= ~(0x01 << bit_position);
    rtc8564_write( addr, data ); 
  } 

  uint8_t rtc8564_dec2bcd(uint8_t data) {
    return ((( data / 10) << 4) + (data % 10));
  }

  uint8_t rtc8564_bcd2dec(uint8_t data) {
    return ((( data >> 4) * 10) + (data % 16));
  }
  
  uint8_t rtc8564_calc_weekday(int year, int month, int day) {
    if( month <= 2 ) {
      month += 12;
      year--;
    }
    
    return (uint8_t)((year + year/4 - year/100 + year/400 + ((13 * month + 8)/5) + day) % 7);
  }
  
  boolean rtc8564_is_power_on() {
    if( rtc8564_test_bit(REG_ADDR_SECONDS, VOLTAGE_LOW ) ) {
      return true;
    } else {
      return false;
    }
  }
};

//-----------------------

RTC8546Wrapper::RTC8546Wrapper() {
}

RTC8546Wrapper::~RTC8546Wrapper() {
}

bool RTC8546Wrapper::Init() {
  Wire.begin();

  disableAlarm();
  disableAlarmInterrupt();
  
  disableTimer();
  disableTimerInterrupt();
  
  if (rtc8564_is_power_on() == true) {
    return true;
  } else {
    return false;
  }
}

bool RTC8546Wrapper::SetTime(const tm &time) {  
  rtc8564_write(REG_ADDR_SECONDS,  rtc8564_dec2bcd(time.tm_sec));
  rtc8564_write(REG_ADDR_MINUTES,  rtc8564_dec2bcd(time.tm_min));  
  rtc8564_write(REG_ADDR_HOURS,    rtc8564_dec2bcd(time.tm_hour));
  rtc8564_write(REG_ADDR_DAYS,     rtc8564_dec2bcd(time.tm_mday));
  rtc8564_write(REG_ADDR_WEEKDAYS, rtc8564_calc_weekday(time.tm_year, time.tm_mon, time.tm_mday));
  rtc8564_write(REG_ADDR_MONTHS,   rtc8564_dec2bcd(time.tm_mon));
  rtc8564_write(REG_ADDR_YEARS,    rtc8564_dec2bcd((uint8_t)(time.tm_year-2000)));
  rtc8564_clear_bit( REG_ADDR_CONTROL1, STOP_BIT );
}

bool RTC8546Wrapper::GetTime(tm *time) {
  time->tm_year  = rtc8564_bcd2dec( rtc8564_read( REG_ADDR_YEARS )) + 2000;
  time->tm_mon   = rtc8564_bcd2dec( rtc8564_read( REG_ADDR_MONTHS ) & MONTHS_MASK );
  time->tm_mday  = rtc8564_bcd2dec( rtc8564_read( REG_ADDR_DAYS ) & DAYS_MASK   );
  time->tm_hour  = rtc8564_bcd2dec( rtc8564_read( REG_ADDR_HOURS    ) & HOURS_MASK  );
  time->tm_min   = rtc8564_bcd2dec( rtc8564_read( REG_ADDR_MINUTES  ) & MINUTES_MASK);
  time->tm_sec   = rtc8564_bcd2dec( rtc8564_read( REG_ADDR_SECONDS  ));
  time->tm_wday  = rtc8564_bcd2dec( rtc8564_read( REG_ADDR_WEEKDAYS ));
  
  return true;
}

bool RTC8546Wrapper::SetWakeupTime(const tm &time) {
  enableAlarm(time);
  enableAlarmInterrupt();
}

bool RTC8546Wrapper::ResetWakeupTime() {
}

void RTC8546Wrapper::ClearWakeupFactor() {
  clearAlarmIrqFlag();
  clearTimerIrqFlag();
}

//------ private

// Alarm Control

void RTC8546Wrapper::enableAlarm(const tm &time) {
  // no care
  rtc8564_set_bit( REG_ADDR_DAY_ALARM,     ALARM_ENABLE );
  rtc8564_set_bit( REG_ADDR_WEEKDAY_ALARM, ALARM_ENABLE );
  
  rtc8564_write(REG_ADDR_HOUR_ALARM, rtc8564_dec2bcd(time.tm_hour));
  rtc8564_write(REG_ADDR_MINUTE_ALARM, rtc8564_dec2bcd(time.tm_min));
}

void RTC8546Wrapper::disableAlarm() {
  rtc8564_set_bit( REG_ADDR_DAY_ALARM,     ALARM_ENABLE );
  rtc8564_set_bit( REG_ADDR_HOUR_ALARM,    ALARM_ENABLE );
  rtc8564_set_bit( REG_ADDR_MINUTE_ALARM,  ALARM_ENABLE );
  rtc8564_set_bit( REG_ADDR_WEEKDAY_ALARM, ALARM_ENABLE );
}

void RTC8546Wrapper::enableAlarmInterrupt() {
  rtc8564_set_bit( REG_ADDR_CONTROL2, ALARM_INTERRUPT_ENABLE );
}

void RTC8546Wrapper::disableAlarmInterrupt() {
  rtc8564_clear_bit( REG_ADDR_CONTROL2, ALARM_INTERRUPT_ENABLE );
}

void RTC8546Wrapper::clearAlarmIrqFlag() {
  rtc8564_clear_bit( REG_ADDR_CONTROL2, ALARM_FLAG );
}

// Timer Control

void RTC8546Wrapper::enableTimer(uint8_t count, uint8_t clock) {
  disableTimer();
  rtc8564_write(REG_ADDR_TIMER_CONTROL, clock );  
  rtc8564_write(REG_ADDR_TIMER, count );
  rtc8564_set_bit( REG_ADDR_TIMER_CONTROL, TIMER_ENABLE );
}

void RTC8546Wrapper::disableTimer() {
  rtc8564_clear_bit( REG_ADDR_TIMER_CONTROL, TIMER_ENABLE );
}

void RTC8546Wrapper::enableTimerInterrupt() {
  rtc8564_set_bit( REG_ADDR_CONTROL2, TIMER_INTERRUPT_ENABLE );
}

void RTC8546Wrapper::disableTimerInterrupt() {
  rtc8564_clear_bit( REG_ADDR_CONTROL2, TIMER_INTERRUPT_ENABLE );
}

void RTC8546Wrapper::clearTimerIrqFlag() {
  rtc8564_clear_bit( REG_ADDR_CONTROL2, TIMER_FLAG );
}

