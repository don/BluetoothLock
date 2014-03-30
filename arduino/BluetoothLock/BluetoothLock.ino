// Bluetooth Low Energy Lock
// (c) 2014 Don Coleman
// 
// Bluefruit LE http://adafru.it/1697
// Solenoid Lock http://adafru.it/1512
// https://github.com/adafruit/Adafruit_nRF8001

#include <SPI.h>
#include "Adafruit_BLE_UART.h"

#define LOCK_PIN 6
#define RED_LED_PIN 7
#define GREEN_LED_PIN 8

// Connect CLK/MISO/MOSI to hardware SPI
// e.g. On UNO & compatible: CLK=13, MISO = 12, MOSI = 11
#define ADAFRUITBLE_REQ 10
#define ADAFRUITBLE_RDY 2 // interrupt pin 2 or 3 on UNO
#define ADAFRUITBLE_RST 9

long secret = 12345;
long openTime = 0;
// Status from the Bluefruit LE driver
int lastStatus = ACI_EVT_DISCONNECTED;

Adafruit_BLE_UART BTLEserial = Adafruit_BLE_UART(ADAFRUITBLE_REQ, ADAFRUITBLE_RDY, ADAFRUITBLE_RST);

void setup() {
  Serial.begin(9600);
  Serial.println(F("BLE Safe - Adafruit Bluefruit Low Energy Edition"));
  BTLEserial.begin();

  pinMode(LOCK_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);  
  digitalWrite(LOCK_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW); 
}

void loop() {

  // Tell the nRF8001 to do whatever it should be working on
  BTLEserial.pollACI();

  int status = BTLEserial.getState();
      
  if (status != lastStatus) {
    if (status == ACI_EVT_DEVICE_STARTED) {
      Serial.println(F("* Advertising Started"));
    } 
    else if (status == ACI_EVT_CONNECTED) {
      Serial.println(F("* Connected!"));
    }     
    else if (status == ACI_EVT_DISCONNECTED) {
      Serial.println(F("* Disconnected or advertising timed out."));
    } 
    // save for next loop
    lastStatus = status;
  }
    
  if (status == ACI_EVT_CONNECTED) {
    
    // see if there's any data from bluetooth
    if (BTLEserial.available()) {
      Serial.print("* ");
      Serial.print(BTLEserial.available());
      Serial.println(F(" bytes available from BTLE"));
    }

    // keeping u + code for compatibility with the serial api
    if (BTLEserial.find("u")) {
      int code = BTLEserial.parseInt();
      openLock(code);
    }
    
  }

  // close lock and reset lights after x seconds
  if (openTime && millis() - openTime > 4000) {
    resetLock();
  }
  
}

void openLock(int code) {
  openTime = millis();  // set even if bad code so we can reset the lights
  if (code == secret) { 
    // open the lock
    Serial.println("Code matches, opening lock");
    digitalWrite(GREEN_LED_PIN, HIGH); 
    digitalWrite(RED_LED_PIN, LOW);     
    digitalWrite(LOCK_PIN, HIGH); // open the lock
    BTLEserial.println("unlocked");    
  } else {
    // bad code, don't open
    Serial.println("Invalid code " + code);
    digitalWrite(RED_LED_PIN, HIGH);
    BTLEserial.println("invalid code");       
  }
}

// closes the lock and resets the lights
void resetLock() { 
  // reset the lights
  digitalWrite(RED_LED_PIN, LOW); 
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(LOCK_PIN, LOW); // close the lock
  BTLEserial.println("locked");
  openTime = 0;
}
