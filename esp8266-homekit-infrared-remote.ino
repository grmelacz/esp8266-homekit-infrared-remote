/*
 * ESP8266 HomeKit Infrared Remote Blaster
 * By Jan Grmela <grmela@gmail.com> in 2022
 * License: GPL v3 or newer
 * Version: 1.0
 * 
 * Based on Arduino-HomeKit-ESP8266 examples by Mixiaoxiao (Wang Bin)
 * and IRremoteESP8266 examples by crankyoldgit (David Conran)
 * 
 * Tested successfully on a $2 WEMOS D1 mini clone
 * with Arduino 1.8.20 in 2/2022
 * 
 * == Functions ==
 * - 1 statefull switch that turn off after a set time after the last turn on event
 * - 2 stateless switches that work as buttons - momentary switches 
 *     (turn immediatelly off once triggered)
 *
 */
#include <Arduino.h>
#include <arduino_homekit_server.h>
#include "wifi_info.h"
#include <IRremoteESP8266.h>
#include <IRsend.h>

/* USER CONFIG */
// IR module PIN (4 = D2)
#define IR_PIN 4
// LED to blink once transmitted (2 = internal LED)
#define LED_PIN 2
// confirmation LED blink & connected clients log interval
#define BLINK_INTERVAL 1000*10 // 10 seconds
// how long should the stateful switch stay enabled
#define TURN_OFF_AFTER_MILLISECONDS 1000*300 // 10 mins

/* GLOBAL VARS */
#define LOG_D(fmt, ...)   printf_P(PSTR(fmt "\n") , ##__VA_ARGS__);
IRsend irsend(IR_PIN);
extern "C" homekit_server_config_t config;
extern "C" homekit_characteristic_t cha_switch_on;
extern "C" homekit_characteristic_t cha_switch_on2;
extern "C" homekit_characteristic_t cha_switch_on3;
static uint32_t next_heap_millis = 0;
static uint32_t next_switch_off_millis = 0;

void setup() {
  Serial.begin(115200);
  wifi_connect(); // in wifi_info.h
  //homekit_storage_reset(); // To remove the previous HomeKit pairing storage when you first run this new HomeKit example
  my_homekit_setup();
  
  irsend.begin();

  // Set the LED default state to off
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
}

void loop() {
  my_homekit_loop();
  delay(10);
}


// This is a statefull switch with auto-turn off every X minutes
void cha_switch_on_setter(const homekit_value_t value) {
  if(value.bool_value) {
    // TURN ON
      
    // reset the timer & do nothing
    next_switch_off_millis = millis() + TURN_OFF_AFTER_MILLISECONDS;
    LOG_D("Reseting switch timer");

    if(cha_switch_on.value.bool_value) {
      // Already ON
      // Do not need to change the state
      homekit_characteristic_notify(&cha_switch_on, cha_switch_on.value);
      return;
    }
  }
  else {
    // TURN OFF

    if(!cha_switch_on.value.bool_value) {
        // Switch is already off & we want to turn it off
        // Do nothing
        homekit_characteristic_notify(&cha_switch_on, cha_switch_on.value);
        return;
    }
  }
  

  // We want to change the state to the other one
  // Update the state and trigger the IR
  cha_switch_on.value.bool_value = !cha_switch_on.value.bool_value;
  irsend.sendNEC(0x807F906F);
  switch_triggered(1);

  // sync the HK state with our variable
  homekit_characteristic_notify(&cha_switch_on, cha_switch_on.value);
}

// These are staless switches (emulated momentary button)
// The setters might be called without the need to send 
// the infrared signal - when the setter is called with 
// null homekit_value_t, the irsend() is not invoked
void cha_switch_on_setter2(const homekit_value_t value) {
  if(!value.is_null) {
    irsend.sendNEC(0x4CB38877);
    switch_triggered(2);
  }
  cha_switch_on2.value.bool_value = false;
  homekit_characteristic_notify(&cha_switch_on2, cha_switch_on2.value);
}

void cha_switch_on_setter3(const homekit_value_t value) {
  if(!value.is_null) {
    irsend.sendNEC(0x4CB328D7);
    switch_triggered(3);
  }
  cha_switch_on3.value.bool_value = false;
  homekit_characteristic_notify(&cha_switch_on3, cha_switch_on3.value);
}

void my_homekit_setup() {
  // Add setters for the switches
  // (could be done more elegantly using an array of function pointers in the future)
  cha_switch_on.setter = cha_switch_on_setter, cha_switch_on2.setter = cha_switch_on_setter2, cha_switch_on3.setter = cha_switch_on_setter3;
  arduino_homekit_setup(&config);


  // Now the push buttons default state (which is always false)
  const homekit_value_t hn = HOMEKIT_NULL_CPP();
  cha_switch_on_setter(hn);
  cha_switch_on_setter2(hn);
  cha_switch_on_setter3(hn);
}

void my_homekit_loop() {
  arduino_homekit_loop();
  const uint32_t t = millis();

  // Periodic LED blink & heap info
  if (t > next_heap_millis) {
    blink_led();
    next_heap_millis = t + BLINK_INTERVAL;
    LOG_D("Free heap: %d, HomeKit clients: %d", ESP.getFreeHeap(), arduino_homekit_connected_clients_count());
    if(cha_switch_on.value.bool_value) {
      LOG_D("Remaining %d secs before turning off switch 1", (next_switch_off_millis - t)/1000);
    }
  }

  // automatically turn off the switch after
  if(cha_switch_on.value.bool_value && t > next_switch_off_millis) {
    const homekit_value_t hn = HOMEKIT_NULL_CPP();
    cha_switch_on_setter(hn);
    LOG_D("Switch turned off after %d seconds", TURN_OFF_AFTER_MILLISECONDS/1000);
  }

}

// Just a console information & LED blink to confirm the switch invocation
void switch_triggered(const int switch_number) {
  LOG_D("Switch no %d triggered!", switch_number);
  
  blink_led();
}

void blink_led(void) {
  digitalWrite(LED_PIN, LOW);
  delay(100);
  digitalWrite(LED_PIN, HIGH);
}
