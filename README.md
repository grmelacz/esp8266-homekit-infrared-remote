# esp8266-homekit-infrared-remote
ESP8266 HomeKit Infrared Remote Blaster

Uses [Arduino-HomeKit-ESP8266](https://github.com/Mixiaoxiao/Arduino-HomeKit-ESP8266) and [IRremoteESP8266](https://github.com/crankyoldgit/IRremoteESP8266).

This is an Arduino sketch that allows you to create HomeKit-integrated infrared blaster to e.g. power on your old television that doesn't support HDMI-CEC, a speaker set that is connected to it or just anything that may be controlled via infrared remote (even an A/C!).

This sketch has 3 switches implemented as an example:
- One stateful switch that turns itself off after 10 minutes
- Two stateless (momentary) switches

*Note: As there is no push button (momentary) switch supported in HomeKit, the stateless switches turn off immediately once they're triggered.*

## How to get it up & running

1. Obtain some ESP8266-based board, infrared sensor & infared transmitter module. All of this could be bought for like $6 and is very easy to put together.
2. First, you need to read your infrared codes (or find them in the docs). For A/Cs, it's slightly more complicated as the codes are usually longer. Use the infrared sensor and [connect it to your ESP board](https://www.makerguides.com/ir-receiver-remote-arduino-tutorial/), then use the reader [demo sketch](https://github.com/crankyoldgit/IRremoteESP8266/tree/master/examples/IRrecvDumpV3). **Make sure to use 3.3V to power the module, not the 5V pin** (this does apply to the transmitter too).
3. Once you get the codes to turn the device on, change volume, etc, you may use them in the `irsend.sendNEC()` (or other manufacturer-appropriate calls) to trigger the required infrared command. See the sketch code.
4. Like you've connected the sensor, you may [connect the transmitter module](https://osoyoo.com/2017/11/05/arduino-lesson-infrared-transmitting-module-and-infrared-receiving-module/) to send out the commands. You may even keep both connected until you finish your project.
5. Once the sketch is uploaded, [follow the instructions](https://github.com/Mixiaoxiao/Arduino-HomeKit-ESP8266#usage) on the Arduino-HomeKit-ESP8266 project page to pair the board with your Apple Home.
6. Now you can control the infrared blaster with your iPhone, iPad, Watch or Siri!

## Function explanation

I'm using the Wemos D1 module with an infrared trasmitter module inside a cool box that my work colleague Jirka have printed for me. The module lays nearby a Genius speaker set that I'm able to turn on and off + control volume (*this may be implemented as a speaker accessory in the future, but it works as it is*).

### The auto turn-off feature

The auto turn-off function is needed as there is currently (as of 2/2022) no easy way to read the Apple TV status (whether it is on or off) and use that to turn the speakers on or off too. But once my TV is turned on, I may use the fact it's USB port is powered. This way I'm able to create a HomeKit sensor (see the [esp8266-homekit-watchdog](https://github.com/grmelacz/esp8266-homekit-watchdog) project) that gets triggered once the ESP module boots up & each 8 minutes. With a bit of HomeKit automation, I'm able to trigger the Remote module once the TV sensor is triggered to keep it on. And once the power from use TV USB goes away, the speakers are automatically turned off after 10 minutes of inactivity.

## Some images

![Running module in the desktop Home App](https://user-images.githubusercontent.com/41264018/154341007-382e067c-13fc-4caa-958a-9e29032b54e2.png)

![Finished module in a box](https://user-images.githubusercontent.com/41264018/154341320-2f735f8d-6c69-4911-bb40-e2b1ff098873.jpg)
