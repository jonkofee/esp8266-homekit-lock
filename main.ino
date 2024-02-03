#include <Arduino.h>
#include <arduino_homekit_server.h>
#include "wifi_info.h"
#include "ESPButton.h"

#define HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_OPEN 0
#define HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_CLOSED 1
#define HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_OPENING 2
#define HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_CLOSING 3
#define HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_STOPPED 4
#define HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_UNKNOWN 255

#define HOMEKIT_CHARACTERISTIC_TARGET_DOOR_STATE_OPEN 0
#define HOMEKIT_CHARACTERISTIC_TARGET_DOOR_STATE_CLOSED 1
#define HOMEKIT_CHARACTERISTIC_TARGET_DOOR_STATE_UNKNOWN 255

#define HOMEKIT_PROGRAMMABLE_SWITCH_EVENT_SINGLE_PRESS   0
#define HOMEKIT_PROGRAMMABLE_SWITCH_EVENT_DOUBLE_PRESS   1
#define HOMEKIT_PROGRAMMABLE_SWITCH_EVENT_LONG_PRESS     2

void setup() {
	Serial.begin(115200);
	wifi_connect();
	my_homekit_setup();
}

void loop() {
	my_homekit_loop();
	delay(10);
}

//==============================
// HomeKit setup and loop
//==============================

// access your HomeKit characteristics defined in my_accessory.c
extern "C" homekit_server_config_t config;
extern "C" homekit_characteristic_t cha_target;
extern "C" homekit_characteristic_t cha_current;

#define PIN_SWITCH 14
#define PIN_LOCK_BUTTON 13

void open_lock() {
  homekit_characteristic_notify(&cha_target, HOMEKIT_UINT8(HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_OPEN));
  homekit_characteristic_notify(&cha_current, HOMEKIT_UINT8(HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_OPEN));

  digitalWrite(PIN_SWITCH, HIGH);
  delay(100);
  digitalWrite(PIN_SWITCH, LOW);

  homekit_characteristic_notify(&cha_target, HOMEKIT_UINT8(HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_CLOSED));
  homekit_characteristic_notify(&cha_current, HOMEKIT_UINT8(HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_CLOSED));
}

void gdo_target_state_set(homekit_value_t new_value) {
  open_lock();
}


void my_homekit_setup() {
	pinMode(PIN_SWITCH, OUTPUT);
  pinMode(PIN_LOCK_BUTTON, INPUT_PULLUP);
	
  digitalWrite(PIN_SWITCH, LOW);

  cha_target.setter = gdo_target_state_set;

	ESPButton.add(0, PIN_LOCK_BUTTON, LOW, true, true);
	ESPButton.setCallback([&](uint8_t id, ESPButtonEvent event) {
    open_lock();
  });

	ESPButton.begin();
  
	arduino_homekit_setup(&config);
}

void my_homekit_loop() {
  ESPButton.loop();
	arduino_homekit_loop();
}
