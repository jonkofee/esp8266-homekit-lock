#include <dummy.h>

// Possible values for characteristic CURRENT_DOOR_STATE:
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

/*
 * switch.ino
 *
 *  Created on: 2020-05-15
 *      Author: Mixiaoxiao (Wang Bin)
 *
 * HAP section 8.38 Switch
 * An accessory contains a switch.
 *
 * This example shows how to:
 * 1. define a switch accessory and its characteristics (in my_accessory.c).
 * 2. get the switch-event sent from iOS Home APP.
 * 3. report the switch value to HomeKit.
 *
 * You should:
 * 1. read and use the Example01_TemperatureSensor with detailed comments
 *    to know the basic concept and usage of this library before other examples。
 * 2. erase the full flash or call homekit_storage_reset() in setup()
 *    to remove the previous HomeKit pairing storage and
 *    enable the pairing with the new accessory of this new HomeKit example.
 */

#include <Arduino.h>
#include <arduino_homekit_server.h>
#include "wifi_info.h"
#include "ESPButton.h"

#define LOG_D(fmt, ...)   printf_P(PSTR(fmt "\n") , ##__VA_ARGS__);

void setup() {
	Serial.begin(115200);
	wifi_connect(); // in wifi_info.h
	//homekit_storage_reset(); // to remove the previous HomeKit pairing storage when you first run this new HomeKit example
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
extern "C" homekit_accessory_t *accessories[];
extern "C" homekit_characteristic_t cha_programmable_switch_event;

uint8_t current_door_state = HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_UNKNOWN;
uint8_t target_door_state = HOMEKIT_CHARACTERISTIC_TARGET_DOOR_STATE_UNKNOWN;
static uint32_t next_heap_millis = 0;

#define PIN_SWITCH 2
#define PIN_BUTTON 0 // Use the Flash-Button of NodeMCU

// Called when the value is read by iOS Home APP
homekit_value_t cha_programmable_switch_event_getter() {
	// Should always return "null" for reading, see HAP section 9.75
	return HOMEKIT_NULL_CPP();
}

void gdo_target_state_set(homekit_value_t new_value) {
  printf("1ajshdjkagsjdhakgskdjasd \n aksdgaskjdhgajksd \n");
  homekit_characteristic_notify(&cha_target, HOMEKIT_UINT8(HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_OPEN));
  homekit_characteristic_notify(&cha_current, HOMEKIT_UINT8(HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_OPEN));

  digitalWrite(PIN_SWITCH, HIGH);
  delay(100);
  digitalWrite(PIN_SWITCH, LOW);

  homekit_characteristic_notify(&cha_target, HOMEKIT_UINT8(HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_CLOSED));
  homekit_characteristic_notify(&cha_current, HOMEKIT_UINT8(HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_CLOSED));
}

void my_homekit_setup() {
	pinMode(PIN_SWITCH, OUTPUT);
	digitalWrite(PIN_SWITCH, LOW);

  pinMode(PIN_BUTTON, INPUT_PULLUP);

  cha_target.setter = gdo_target_state_set;

  pinMode(PIN_BUTTON, INPUT_PULLUP);
	ESPButton.add(0, PIN_BUTTON, LOW, true, true);
	ESPButton.setCallback([&](uint8_t id, ESPButtonEvent event) {
		// Only one button is added, no need to check the id.
		LOG_D("Button Event: %s", ESPButton.getButtonEventDescription(event));
		uint8_t cha_value = 0;
		if (event == ESPBUTTONEVENT_SINGLECLICK) {
			cha_value = HOMEKIT_PROGRAMMABLE_SWITCH_EVENT_SINGLE_PRESS;
		} else if (event == ESPBUTTONEVENT_DOUBLECLICK) {
			cha_value = HOMEKIT_PROGRAMMABLE_SWITCH_EVENT_DOUBLE_PRESS;
		} else if (event == ESPBUTTONEVENT_LONGCLICK) {
			cha_value = HOMEKIT_PROGRAMMABLE_SWITCH_EVENT_LONG_PRESS;
		}
		cha_programmable_switch_event.value.uint8_value = cha_value;
		homekit_characteristic_notify(&cha_programmable_switch_event,
				cha_programmable_switch_event.value);
	});
	ESPButton.begin();

	cha_programmable_switch_event.getter = cha_programmable_switch_event_getter;
  
	arduino_homekit_setup(&config);
}

void my_homekit_loop() {
  ESPButton.loop();
	arduino_homekit_loop();
	const uint32_t t = millis();
	if (t > next_heap_millis) {
		// show heap info every 5 seconds
		next_heap_millis = t + 5 * 1000;
		LOG_D("Free heap: %d, HomeKit clients: %d",
				ESP.getFreeHeap(), arduino_homekit_connected_clients_count());

	}
}
