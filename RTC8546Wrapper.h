//
//  Copyright (c) 2018 Hirotaka Yuno <create.future.technology@gmail.com>.  All right reserved.
//

#ifndef RTC8564WRAPPER_H_
#define RTC8564WRAPPER_H_

#include <time.h>
#include "AbstractRtc.h"

class RTC8546Wrapper : public AbstractRtc {
 public:
  RTC8546Wrapper();
  virtual ~RTC8546Wrapper();
  RTC8546Wrapper(const RTC8546Wrapper&) = delete;
  RTC8546Wrapper& operator=(const RTC8546Wrapper&) = delete;
  
  virtual bool Init() final;
  virtual bool SetTime(const tm &time) final;
  virtual bool GetTime(tm *time) final;
  virtual bool SetWakeupTime(const tm &time) final;
  virtual bool ResetWakeupTime() final;
  virtual void ClearWakeupFactor() final;

 private:
  // Alarm Contorol
  void enableAlarm(const tm &time);
  void disableAlarm();
  void enableAlarmInterrupt();
  void disableAlarmInterrupt();
  void clearAlarmIrqFlag();

  // Timer Contorol
  void enableTimer(uint8_t count, uint8_t clock);
  void disableTimer();
  void enableTimerInterrupt();
  void disableTimerInterrupt();
  void clearTimerIrqFlag();
};

#endif  // RTC8564WRAPPER_H_

