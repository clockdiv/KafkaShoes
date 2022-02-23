/* SENDER
   ...is a ESP32 Device
*/
#define LED_RED 2
#define LED_GREEN A17
#define POTI_PIN_THROTTLE A0  // throttlePotixis: throttle
#define POTI_PIN_STEERING A3  // yAxis: steering
#define POTI_STEERING_ADJUST A4

#include <esp_now.h>  // the esp32 version *does* have a underscore, the esp8266 version is espnow.h
#include <WiFi.h>
#include "averageFilter.h"


// Receiver MAC Address e.g. BC:DD:C2:17:5E:3D 
//uint8_t broadcastAddress[] = {0xCC, 0x50, 0xE3, 0x60, 0xF0, 0x40}; // 'A'
//uint8_t broadcastAddress[] = {0xCC, 0x50, 0xE3, 0x60, 0xE4, 0xF7}; // 'B'
//uint8_t broadcastAddress[] = {0xBC, 0xDD, 0xC2, 0xAC, 0x64, 0x37}; // 'C'
//uint8_t broadcastAddress[] = {0xCC, 0x50, 0xE3, 0x60, 0xF6, 0x36}; // 'D'
//uint8_t broadcastAddress[] = {0xCC, 0x50, 0xE3, 0x60, 0xE9, 0xD7}; // 'E'
//uint8_t broadcastAddress[] = {0xBC, 0xDD, 0xC2, 0xAC, 0x6B, 0xD9}; // 'F'
//uint8_t broadcastAddress[] = {0xCC, 0x50, 0xE3, 0x60, 0xEA, 0x4B}; // 'G'
//uint8_t broadcastAddress[] = {0xCC, 0x50, 0xE3, 0x6A, 0x89, 0x92}; // 'H'
uint8_t broadcastAddress[] = {0xCC, 0x50, 0xE3, 0x60, 0xF3, 0xD5}; // 'I' keine Akkus in Schuh und Remote; Achse gebrochen
//uint8_t broadcastAddress[] = {0xBC, 0xDD, 0xC2, 0x17, 0x5E, 0x3D}; // 'J'

typedef struct motor_struct {
  int motorLeft;
  int motorRight;
} motor_struct;

motor_struct motorData, motorDataOld;

averageFilter avX, avY;


uint16_t centerThrottle = 0;
uint16_t centerSteering = 0;
bool isCalibrated = false;
unsigned long currentMillis, millisOldMessageSent;
unsigned long messageSendInterval = 40;
int tolerance = 10;

// ------------------------------------------------
// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  //Serial.print("Packet to: ");

  // Copies the sender mac address to a string
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  // Serial.print(macStr);
  // Serial.print(" send status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");

  // digitalWrite(LED_RED, !ESP_NOW_SEND_SUCCESS);
  digitalWrite(LED_GREEN, HIGH);
}


// ------------------------------------------------
void setup() {
  Serial.begin(115200);
  pinMode(LED_RED, OUTPUT);       // ESP8266 LED - 2, D4, LED_ESP
  pinMode(LED_GREEN, OUTPUT);

  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, LOW);  // Turn ESP's tiny LED off at startup (anyway, that pin is used for the tb6612fng


  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  // register peer
  esp_now_peer_info_t peerInfo;
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // register  peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}

// ------------------------------------------------
void loop() {
  currentMillis = millis();
  // read potis
  int16_t potiThrottle = analogRead(POTI_PIN_THROTTLE); // throttlePotixis: throttle
  int16_t potiSteering = analogRead(POTI_PIN_STEERING); // yAxis: steering

  if (!isCalibrated)
  {
    digitalWrite(LED_GREEN, HIGH);
    delay(100);
    centerThrottle = potiThrottle;
    centerSteering = potiSteering;
    digitalWrite(LED_GREEN, LOW);
    isCalibrated = true;
  }
  else {
    // map potis from center to 1 and -1
    potiThrottle = avX.filter(potiThrottle);
    potiSteering = avY.filter(potiSteering);

    int throttle = (potiThrottle > centerThrottle) ? map(potiThrottle, centerThrottle, 4095, 0, 127) : map(potiThrottle, centerThrottle, 0, 0, -127);
    int steering = (potiSteering > centerSteering) ? map(potiSteering, centerSteering, 4095, 0, 127) : map(potiSteering, centerSteering, 0, 0, -127);

    throttle *= -1;

    if (currentMillis - millisOldMessageSent > messageSendInterval) {
      //digitalWrite(LED_RED, LOW);

      if (abs(throttle) - tolerance < 0) throttle = 0;
      if (abs(steering) - tolerance < 0) steering = 0;

      motorData.motorLeft = constrain(throttle + steering, -127, 127);
      motorData.motorRight = constrain(throttle - steering, -127, 127);
      digitalWrite(LED_GREEN, LOW);
      //Serial.printf("%i,%i\r\n", xAxis, yAxis);
      //    drive(int(m2), int(m1));

      if (motorData.motorLeft != motorDataOld.motorLeft ||
          motorData.motorRight != motorDataOld.motorRight)
      {
        esp_err_t result = esp_now_send(0, (uint8_t *) &motorData, sizeof(motor_struct));
        if (result == ESP_OK) {
          //Serial.println("Sent with success");
        }
        else {
          //Serial.println("Error sending the data");
        }
      }
      millisOldMessageSent = currentMillis;
    }
  }




}
