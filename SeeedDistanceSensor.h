//
//  Copyright (c) 2018 Hirotaka Yuno <create.future.technology@gmail.com>.  All right reserved.
//

#ifndef DISTANCE_SENSOR_SOURCE_H_
#define DISTANCE_SENSOR_SOURCE_H_

#include "AbstractDataSource.h"

class SeeedDistanceSensor : public AbstractDataSource
{
 public:
  SeeedDistanceSensor() = default;
  virtual ~SeeedDistanceSensor() = default;

  virtual void Init() final;
  virtual void GetData(st_CollectedData *data) final;

 private:
  int pin_;
  double duration_time_;
  double duration_;
  double previous_;
  boolean is_first_;
  double distance_;
};

#endif
