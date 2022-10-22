# ESP32-Watch For OpenHaystack

[![Demo](https://github.com/0015/ESP32-Watch-Projects/blob/main/ESP32-Watch-For-OpenHaystack/demo/watch-demo.gif)](https://youtu.be/8672iWVhBvM)

It's able to track the ESP32 without GPS using OpenHaystack. Because it works so well, it may lead to privacy issues.

The ESP32-Watch for OpenHaystack is designed to be trackable by the user's choice. It includes minimal watch functionality and broadcasting over BLE for Apple's Find my network.

## Introduction

This project is based on the firmware for ESP32 from OpenHaystack(https://github.com/seemoo-lab/openhaystack)

## OpenHaystack

To discover your device through Apple's Fine my network, you must first run OpenHaystack on your Mac. For more information, please refer to the OpenHaystack Github.

## OpenHaystack Firmware for ESP32

https://github.com/seemoo-lab/openhaystack/tree/main/Firmware/ESP32


## ESP-IDF Environment

First of all, the ESP-IDF development environment should be ready in advance.
[Tutorials for the ESP-IDF Visual Studio Code Extension](https://github.com/espressif/vscode-esp-idf-extension/blob/master/docs/tutorial/toc.md)

## Hardware

For the ESP32-Watch, I used the ESP32-PICO-D4, but any ESP32 would work.
- Any ESP32 (not ESP32-S2)
- SPI interface Display or more
- 2 Buttons
- LiPo Battery

## Software

Libraries used in ESP32 Watch are as follows.
- LVGL(https://github.com/lvgl/lvgl)
- LovyanGFX(https://github.com/lovyan03/LovyanGFX)
- ESP32-Button(https://github.com/craftmetrics/esp32-button)


### Created & Maintained By

[Eric Nam](https://github.com/0015)
([Youtube](https://youtube.com/ThatProject))
([Facebook](https://www.facebook.com/groups/138965931539175))]


### MIT License

Copyright (c) 2022 Eric

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.