//
//  Copyright (c) 2018 Hirotaka Yuno <create.future.technology@gmail.com>.  All right reserved.
//

#include "CommonResource.h"
#include "NetworkManager.h"

namespace {
  void GenerateClientId(char *id) {
    long num = random(100000);
    char tmp_id[ConstantValue::CLIENT_ID_SIZE] = {0};
    String client = String(num);
    
    SerialUSB.print("client ID: ");
    SerialUSB.println(client);
    client.toCharArray(tmp_id, ConstantValue::CLIENT_ID_SIZE);
    memcpy(id, tmp_id, ConstantValue::CLIENT_ID_SIZE);
  }
}

NetworkManager::NetworkManager(WioLTE *wio, WioLTEClient *wio_client, PubSubClient *mqtt_client) :
  wio_(wio), wio_client_(wio_client), mqtt_client_(mqtt_client){
}

NetworkManager& NetworkManager::GetInstance() {
  static WioLTE wio;
  static WioLTEClient wio_client(&wio);
  static PubSubClient mqtt_client;
  static NetworkManager instance(&wio, &wio_client, &mqtt_client);
  return instance;
}

bool NetworkManager::SystemInit() {
  wio_->Init();
  return true;
}

bool NetworkManager::InitGrove() {
  wio_->Init();
  wio_->PowerSupplyGrove(true);
  return true;
}

// LTE繝｢繧ｸ繝･繝ｼ繝ｫ繧貞�晄悄蛹悶☆繧�
bool NetworkManager::InitNetwork() {  
  wio_->PowerSupplyLTE(true);
  delay(500);  // Wait for wake up LTE module.
  
  if (!wio_->TurnOnOrReset()) {
    // init error;
    SerialUSB.println("TurnOnOrReset() error");
    return false;
  }
  SerialUSB.println("TurnOnOrReset() success");

  delay(500);
  
  return true;
}

// LTE騾壻ｿ｡繧貞��譁ｭ縺吶ｋ
bool NetworkManager::Exit() {
  //  if (mqtt_client_->connected() == true) {
  //  mqtt_client_->disconnect();
  //}
  wio_->Deactivate();  // Deactivate a PDP context. Added at v1.1.9
  wio_->TurnOff(); // Shutdown the LTE module. Added at v1.1.6
  wio_->PowerSupplyLTE(false); // Turn the power supply to LTE module off
  wio_->PowerSupplyGrove(false);
}

// NTP繧ｵ繝ｼ繝舌°繧画凾蛻ｻ(UTC)繧貞叙蠕励☆繧�
bool NetworkManager::GetNtpTime(tm *p_current_time) {
  if(!wio_->SyncTime(ConstantValue::NTP_SERVER)) {
    SerialUSB.println("SyncTime() error");
    return false;
  }
  return wio_->GetTime(p_current_time);
}

// PDP繧ｳ繝ｳ繝�繧ｯ繧ｹ繝医ｒ繧｢繧ｯ繝�繧｣繝吶�ｼ繝医☆繧�
bool NetworkManager::Activate() {
  if (!wio_->Activate(ConstantValue::APN,
                      ConstantValue::USERNAME,
                      ConstantValue::PASSWORD)) {
    // Activate error.
    SerialUSB.println("Activate() error");
    return false;
  }
  SerialUSB.println("Activate() success");
  return true;
}

bool NetworkManager::GetPhoneNumber(char *number, const int &num) {
  if (wio_->GetPhoneNumber(number, num) < 0) {
    return false;
  }
  return true;
}

bool NetworkManager::GetIMSI(char *number, const int &num) {
  if (wio_->GetIMSI(number, num) < 0) {
    return false;
  }
  return true;
}

// MQTT繧ｵ繝ｼ繝舌∈謗･邯壹☆繧�
bool NetworkManager::MqttConnect() {
  char client_id[ConstantValue::CLIENT_ID_SIZE] = {0};

  mqtt_client_->setServer(ConstantValue::MQTT_SERVER_HOST,
                          ConstantValue::MQTT_SERVER_PORT);
  
  mqtt_client_->setClient(*wio_client_);
  
  GenerateClientId(client_id);
  if (!mqtt_client_->connect(client_id)) {
    SerialUSB.println("connect() error");
    return false;
  }
  SerialUSB.println("connect() success");
  return true;
}

// 繝�繝ｼ繧ｿ繧恥ublish縺吶ｋ
bool NetworkManager::MqttPublish(const char *json) {
  return mqtt_client_->publish(ConstantValue::TOPIC_NAME, json);
}

