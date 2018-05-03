//
//  Copyright (c) 2018 Hirotaka Yuno <create.future.technology@gmail.com>.  All right reserved.
//

#ifndef ABSTRACT_RTC_H_
#define ABSTRACT_RTC_H_

#include <time.h>

class AbstractRtc {
 public:
  AbstractRtc(){};
  virtual ~AbstractRtc(){};
  AbstractRtc(const AbstractRtc&) = delete;
  AbstractRtc& operator=(const AbstractRtc&) = delete;
  
  virtual bool Init() = 0;
  virtual bool SetTime(const tm &time) = 0;
  virtual bool GetTime(tm *time) = 0;
  virtual bool SetWakeupTime(const tm &time) = 0;
  virtual bool ResetWakeupTime() = 0;
  virtual void ClearWakeupFactor() = 0;
};

#endif  // ABSTRACT_RTC_H_

