//
//  Copyright (c) 2018 Hirotaka Yuno <create.future.technology@gmail.com>.  All right reserved.
//

#ifndef ABSTRACT_DATA_SOURCE_H_
#define ABSTRACT_DATA_SOURCE_H_

#include <time.h>
#include <Arduino.h>

typedef struct st_CollectedData {
  //uint32_t device_id;
  char device_id[32];
  time_t timestamp;
  double distance;
}st_CollectedData;

class AbstractDataSource {
 public:
  AbstractDataSource(){};
  virtual ~AbstractDataSource(){};
  AbstractDataSource(const AbstractDataSource&) = delete;
  AbstractDataSource& operator=(const AbstractDataSource&) = delete;
  
  virtual void Init() = 0;
  virtual void GetData(st_CollectedData *data) = 0;
};

#endif  // ABSTRACT_DATA_SOURCE_H_

