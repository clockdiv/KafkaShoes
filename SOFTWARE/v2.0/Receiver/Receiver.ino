/* RECEIVER
   ...is a ESP8266 Device
*/
#define LED_MESSAGE_RECEIVED D0 // "WiFi"-labeled LED green
#define LED_WIFI_OK D4          // "WiFi"-labeled LED red
#define LED_BATTERY_OK 10       // "Batt"-labeled LED green
#define LED_BATTERY_LOW 9       // "Batt"-labeled LED red

#include <espnow.h> // the esp8266 version does *not* have a underscore, the esp32 version is esp_now.h
#include <ESP8266WiFi.h>
#include "TB6612FNG.h"


typedef struct motor_struct {
  int x;
  int y;
} motor_struct;

motor_struct motorData;

tb6612fng motors;

unsigned long autoStopResetInterval = 100;

unsigned long lastTimeAutoStopReset;
unsigned long currentMillis;




// ------------------------------------------------
//callback function that will be executed when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  digitalWrite(LED_MESSAGE_RECEIVED, HIGH);
  memcpy(&motorData, incomingData, sizeof(motorData));
  /*
    Serial.print("Bytes received: ");
    Serial.println(len);
    Serial.print("x: ");
    Serial.println(motorData.x);
    Serial.print("y: ");
    Serial.println(motorData.y);
    Serial.println();
  */
  motors.drive(motorData.x, motorData.y);
}


// ------------------------------------------------
void setup() {
  analogWriteRange(1023);
  
  pinMode(LED_WIFI_OK, OUTPUT);
  pinMode(LED_MESSAGE_RECEIVED, OUTPUT);
  pinMode(LED_BATTERY_OK, OUTPUT);
  pinMode(LED_BATTERY_LOW, OUTPUT);

  digitalWrite(LED_WIFI_OK, LOW);
  digitalWrite(LED_MESSAGE_RECEIVED, LOW);
  digitalWrite(LED_BATTERY_OK, LOW);
  digitalWrite(LED_BATTERY_LOW, LOW);

  Serial.begin(115200);
  Serial.println();
  Serial.print("ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());

  WiFi.mode(WIFI_STA);

  //Init ESP-NOW
  if (esp_now_init() != 0) {
    //Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);


  motors.initPins();
  currentMillis = millis();
  lastTimeAutoStopReset = currentMillis;
}

// ------------------------------------------------
void loop() {
  // auto-stop if no signal received (e.g. if remote or wifi breaks)
  currentMillis = millis();

  if (currentMillis - lastTimeAutoStopReset > autoStopResetInterval)
  {
    motors.stop();
    lastTimeAutoStopReset = currentMillis;
  }

}
