//
//  Copyright (c) 2018 Hirotaka Yuno <create.future.technology@gmail.com>.  All right reserved.
//

#include <WioLTEforArduino.h>
#include "Arduino.h"
#include "SeeedDistanceSensor.h"

#define SENSOR_PIN (WIOLTE_D38) // Echo Pin

namespace {
const int kMeasurementCount = 5;
uint32_t MicrosDiff(uint32_t begin, uint32_t end)
{
  return end - begin;
}

uint32_t pulseIn(uint32_t pin, uint32_t state, uint32_t timeout = 1000000L)
{
  uint32_t begin = micros();
  
  // wait for any previous pulse to end
  while (digitalRead(pin)) {
    if (MicrosDiff(begin, micros()) >= timeout) return 0;
  }
  
  // wait for the pulse to start
  while (!digitalRead(pin)) {
    if (MicrosDiff(begin, micros()) >= timeout) return 0;
  }

  uint32_t pulseBegin = micros();
	
  // wait for the pulse to stop
  while (digitalRead(pin)) {
    if (MicrosDiff(begin, micros()) >= timeout) return 0;
  }
  
  uint32_t pulseEnd = micros();
	
  return MicrosDiff(pulseBegin, pulseEnd);
}  
};

void SeeedDistanceSensor::Init() {
  pin_ = SENSOR_PIN;
  SerialUSB.println("Seeed Sensor Init");
}

void SeeedDistanceSensor::GetData(st_CollectedData *data) {
  // 5回測定し、最大値と最小値を除いた3回分のデータの平均を取る
  double tmp[kMeasurementCount] = {0};
  double duration = 0;
  double sum = 0;
  double max = -1;
  double min = 100000000;

  SerialUSB.println("Seeed Sensor GetData");
  
  // 5回分のデータを取得
  for (int i = 0; i < kMeasurementCount; i++) {
    pinMode(pin_, OUTPUT);
    digitalWrite(pin_, LOW);
    delayMicroseconds(2);
    digitalWrite(pin_, HIGH);
    delayMicroseconds(5);
    digitalWrite(pin_,LOW);
    pinMode(pin_,INPUT);

    duration = pulseIn(pin_,HIGH);
  
    if (duration > 0) {
      duration = duration/2; //往復距離を半分にする
      tmp[i] = duration*340*100/1000000; // 音速を340m/sに設定
      sum += tmp[i]; // 後で平均を出すので、加算しておく
      SerialUSB.println(tmp[i]);
    }
    delay(100);
  }

  // 最大値と最小値を算出
  for (int i = 0; i < kMeasurementCount; i++) {
    if (tmp[i] > max) {
      max = tmp[i];
    }
    if (tmp[i] < min) {
      min = tmp[i];
      
    }
  }

  // 平均値を算出し、結果を格納
  data->distance = (sum - max - min) / (kMeasurementCount - 2);
}
