//
//  Copyright (c) 2018 Hirotaka Yuno <create.future.technology@gmail.com>.  All right reserved.
//

#include <ArduinoJson.h>
#include <RTClock.h>
#include <time.h>
#include <Wire.h>
#include "AbstractDataSource.h"
#include "CommonResource.h"
#include "SeeedDistanceSensor.h"
#include "EnergyManager.h"
#include "MainTask.h"
#include "NetworkManager.h"
#include "PowerCtrl.h"
#include "RTC8546Wrapper.h"

SeeedDistanceSensor dist_sen;

RTC8546Wrapper rtc;

namespace {
  void SerializeToJson(const st_CollectedData &data, char *json, const size_t &size) {
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["device_id"] = data.device_id;
    root["timestamp"] = data.timestamp;
    root["sensor"] = "distance";
    root["data"] = data.distance;
    root.printTo(json, size);
  }
}

MainTask::MainTask() :
  state_(kMeasurement),
  net_mgr_(NULL),
  eng_mgr_(NULL),
  current_time_{0} {
}

void MainTask::Init(NetworkManager *net_mgr, EnergyManager *eng_mgr) {
  net_mgr_ = net_mgr;
  eng_mgr_ = eng_mgr;
  for (int i = 0; i < ConstantValue::DATA_SOURCE_NUM_MAX; i++) {
    data_source_[i] = NULL;
  }

  data_source_[0] = &dist_sen;
  
  for (int i = 0; i < ConstantValue::DATA_SOURCE_NUM_MAX; i++) {
    if (data_source_[i] != NULL) {
      data_source_[i]->Init();
    } else {
      break;
    }
  } 
}

// the setup routine runs once when you press reset:
void MainTask::setup() {
  struct tm current_time = {0};
  SerialUSB.println("MainTask Setup");
  net_mgr_->SystemInit();
  net_mgr_->InitGrove();
  rtc.Init();
  delay(1000);
}

// the loop routine runs over and over again forever:
void MainTask::loop() 
{
  static st_CollectedData data = {0};
  static char json[ConstantValue::JSON_MESSAGE_SIZE] = {0};
  char tmp_num[32] = {0};
  time_t epoch = 0;
  
  // State Machine
  switch (state_) {
    case kMeasurement:
      //
      for (int i = 0; i < ConstantValue::DATA_SOURCE_NUM_MAX; i++) {
        data_source_[i]->GetData(&data);
        SerialUSB.print("Data SensorID ");
        SerialUSB.print(i);
        SerialUSB.print(" : ");
        SerialUSB.println(data.distance);

        if ((i < ConstantValue::DATA_SOURCE_NUM_MAX - 1) && (data_source_[i+1] == NULL)) {          
          break;
        }
      }
      
      state_ = kPublish;
      break;
      
    case kPublish:
      net_mgr_->InitNetwork();
      net_mgr_->Activate();
      net_mgr_->GetIMSI(tmp_num, sizeof(tmp_num));
      SerialUSB.print("IMSI : ");
      SerialUSB.println(tmp_num);
      strncpy(data.device_id, tmp_num, sizeof(tmp_num));

      if (net_mgr_->GetNtpTime(&current_time_) == true) {        
        SerialUSB.print("Connet NTP Setver Success, UTC : ");
        epoch = mktime(&current_time_);  
        epoch += ConstantValue::JAPAN_TIME_DIFF;
        current_time_ = *gmtime(&epoch);
        rtc.SetTime(current_time_);
        delay(10);
      } else {
        rtc.GetTime(&current_time_);
        SerialUSB.print("[Error] Connet NTP Setver Failed, UTC : ");
      }

      SerialUSB.println(asctime(&current_time_));
      data.timestamp = mktime(&current_time_);
      net_mgr_->MqttConnect();
      SerializeToJson(data, json, ConstantValue::JSON_MESSAGE_SIZE);
      SerialUSB.println(json);

      if (net_mgr_->MqttPublish(json)) {
        SerialUSB.println("Publish Completed");
      } else {
        SerialUSB.println("Publish Failed");
      }

      tm dbg_t;
      rtc.GetTime(&dbg_t);
      SerialUSB.print("dbg_t: ");
      SerialUSB.println(asctime(&dbg_t));

      delay(500);
      
      state_ = kExit;
      break;

    case kExit:
    default:
      struct tm wakeup_time = {0};
      
      epoch = mktime(&current_time_);
      epoch += ConstantValue::WAKE_UP_INTERVAL;
      
      wakeup_time = *gmtime(&epoch);
      SerialUSB.print("Wakeup Time:");
      SerialUSB.println(asctime(&wakeup_time));
 
      rtc.SetWakeupTime(wakeup_time);
      delay(100);
      rtc.ClearWakeupFactor();
      
      net_mgr_->Exit();
      while(1); // システムが終了するまでループ
      break;
  }
}

