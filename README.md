# Lego-G-Sensor-Display
Non-mbed Seeed Xiao nRF52840 Sense (with accelerometer) communicating with a second Seeed Xiao nRF52840 Sense via Bluetooth (Bluefruit) which displays the acceleration and severity (Lego facial expression) indicator on a Seeed Round Display. 

The project includes minimum working examples for various parts of the project and full code for both boards.

Board 1 (**Sensor Assembly** - Bluetooth Peripheral):
- Regularly record accelerometer data
- Transmit data to Board 2.

Board 2 (**Display Assembly** - Bluetooth Central):
- Receive data from Board 1.
- Display accelerometer data and different Lego Minifigure facial expression based on acceleration severity.

This project is for my dad who designs Lego amusement park rides [afol_michel on Instagram](https://www.instagram.com/afol_michel/). The goal was to make the smallest possible microcontroller accelerometer package that can transmit results to an external display. The idea was to have the accelerometer mounted in a ride like a Minifigure passenger, with the display showing their current enjoyment level (i.e. facial expression).

Currently, there are 6 facial expressions available in the software (all inspired by actual Lego Minifigure expressions):
- Happy Face
- Sweating Face
- Worried Face
- Crying Face
- Puking Face
- Dead Face


# Software Setup and Use
The documentation to get the Seeed Xiao BLE nRF25840 Sense and Xiao Round Display is quite fragmented and hard to navigate with provided examples that don't work out of the box or only work if the boards are set as Mbed or Non-Mbed. 

To get Seeed boards recognized by the Arduino IDE, you need to add the following URL in the list of Additional Boards Manager URLs: https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json
Then, you can install either of the two sets of board definitions in the Board Manager. The name of the board you must select is different depending on which version you are using. This project requires the non-mBED version.
- Seeed nRF52 mbed-enabled Boards (Seeed Xiao BLE - nRF52840)
- **USE THIS ONE:** Seeed nRF52 Boards (Seeed Xiao nRF52840 Sense)

Note, the tutorials on the Seeed Wiki require different board versions here are the options:
IMU:
- For Mbed boards: Follow the official tutorial [Seeed Tutorial](https://wiki.seeedstudio.com/XIAO-BLE-Sense-IMU-Usage/) which uses the [Seeed_Arduino_LSM6DS30](https://github.com/Seeed-Studio/Seeed_Arduino_LSM6DS3) Library.
- **USE THIS ONE:** For non-Mbed boards, use the forked version by pdcook: [Seeed_LSM6DS3_non_mbed](https://github.com/pdcook/Seeed_LSM6DS3_non_mbed) Library. This version renames 'Wire' as 'Wire1' in LSM6DS3.cpp.
  
Bluetooth:
- For Mbed boards: Follow the official tutorial [Seeed Tutorial](https://wiki.seeedstudio.com/XIAO-BLE-Sense-Bluetooth_Usage/) which uses the [ArduinoBLE]([https://github.com/Seeed-Studio/Seeed_Arduino_LSM6DS3](https://github.com/arduino-libraries/ArduinoBLE)) Library.
- **USE THIS ONE:** For non-Mbed boards, use [Adafruit's Bluefruit](https://github.com/adafruit/Adafruit_nRF52_Arduino/tree/master/libraries/Bluefruit52Lib) Library. Documentation and examples are much easier to find for this library. I used various infor from the following guides: [Adafruit Circuit Playground Bluefruit](https://cdn-learn.adafruit.com/downloads/pdf/adafruit-circuit-playground-bluefruit.pdf) and [Bluefruit nRF52 Feather Learning Guide](https://learn.adafruit.com/bluefruit-nrf52-feather-learning-guide/blecharacteristic).

Round Display:
- For non-Mbed boards: No info.
- **USE THIS ONE:** For non-Mbed boards, follow the official [Seeed Setup Guide](https://wiki.seeedstudio.com/get_start_round_display/) and [Seeed Tutorial](https://wiki.seeedstudio.com/seeedstudio_round_display_usage/) which uses the [Seeed_Arduino_RoundDisplay](https://github.com/Seeed-Studio/Seeed_Arduino_RoundDisplay), [SeeedStudio_lvgl](https://github.com/Seeed-Projects/SeeedStudio_lvgl), and [SeeedStudio_TFT_eSPI](https://github.com/Seeed-Projects/SeeedStudio_TFT_eSPI) Libraries.

I had some intermittent issues with the [Arduino IDE](https://www.arduino.cc/en/software). [Version 3.2.1](https://github.com/arduino/arduino-ide/releases) on Windows worked for me. 

Sometimes uploading to the boards fails for some reason (or hangs up). It this happens, double-tap the reset button on the boards, then select the newly changed COM port. Re-upload and it should work. You may need to close the Serial Monitor if issues persist.

# Hardware
Required Components:
- 2x [Seeed Xiao BLE Sense nRF52840](https://www.seeedstudio.com/Seeed-XIAO-BLE-Sense-nRF52840-p-5253.html)
- 1x [Seeed Studio Round Display for XIAO](https://www.seeedstudio.com/Seeed-Studio-Round-Display-for-XIAO-p-5638.html)
- 1x Small Li-Poly battery
- 1x Small SPST Toggle Switch
- 1x External 5V power supply with USB C

Setup:
- **Board 1** had an Li-Poly battery soldered on the back with a small SPST toggle switch on the positive line. The small SPST toggle must be set to On when using or charging the device.
- **Board 2** must be plugged in to the Round Display. An external power supply must be plugged into Board 2. The On/Off switch on the Round Display must be set to On to receive power from the board. Otherwise, it does have a battery port on the back (not used here).

