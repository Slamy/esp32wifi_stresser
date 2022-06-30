# esp32wlan_stresser

## What is this?
This is a stress test for an ESP32 to check how many packages are lost during WLAN transmission.

## How?
This is a combination of a PC Tool and a firmware for the ESP32.
The PC Tool sends packages with a fixed size of 1000 byte to the ESP32 via UDP.
The ESP32 receives these and sends an answer on every received package.
A total of 1000 packages are sent by the PC.

Every package is numbered, so both the PC and the ESP32 are capable of detecting lossed frames.
UDP will not retransmit the lost frames, which is essential for this test.

## Why?

I got hold of [this ESP32 board](https://heltec.org/project/wifi-kit-32/) as my first and it wasn't really reliable when it comes to WLAN.
I wanted to have a tool on my hand to create statistics on the reliability of different ESP32 boards.

## Example use

Flash the software to your ESP32 board. Ensure correct configuration and connect the ESP32 to your PC to monitor the first test.

The serial log of the ESP32 will show its own IP like so.

    I (1597) esp_netif_handlers: sta ip: 192.168.178.47, mask: 255.255.255.0, gw: 192.168.178.1

Before starting the test, use ping to ensure at least some sort of connection.
Then do this:

    $ ./espstresser 192.168.178.47
    RecvFrameCnt:1 LostEspToPc:0 EspRssi:-76 LostFramesPcToEsp:0   0 0 0 0
    RecvFrameCnt:2 LostEspToPc:0 EspRssi:-76 LostFramesPcToEsp:0   0 0 0 0
    ...
    RecvFrameCnt:996 LostEspToPc:0 EspRssi:-78 LostFramesPcToEsp:0   0 0 0 0
    RecvFrameCnt:997 LostEspToPc:0 EspRssi:-78 LostFramesPcToEsp:0   0 0 0 0
    RecvFrameCnt:998 LostEspToPc:0 EspRssi:-78 LostFramesPcToEsp:0   0 0 0 0
    RecvFrameCnt:999 LostEspToPc:0 EspRssi:-78 LostFramesPcToEsp:0   0 0 0 0
    Waiting for remaining frames ...
    RecvFrameCnt:1000 LostEspToPc:0 EspRssi:-78 LostFramesPcToEsp:0   0 0 0 0
    Finished Test. Total Lost Ping Pong Cycles: 0

This was an actual perfect transmission. Nothing was lost on this [ESP32 NodeMCU](https://www.berrybase.de/esp32-nodemcu-development-board).

Now let's do it again with the [WiFi Kit 32](https://heltec.org/project/wifi-kit-32/)

    $ ./espstresser 192.168.178.37
    RecvFrameCnt:11 LostEspToPc:0 EspRssi:-66 LostFramesPcToEsp:0   0 0 0 0
    RecvFrameCnt:12 LostEspToPc:0 EspRssi:-66 LostFramesPcToEsp:0   0 0 0 0
    ...
    RecvFrameCnt:997 LostEspToPc:352 EspRssi:-72 LostFramesPcToEsp:0   0 203 0 0
    RecvFrameCnt:998 LostEspToPc:352 EspRssi:-72 LostFramesPcToEsp:0   0 203 0 0
    Waiting for remaining frames ...
    RecvFrameCnt:999 LostEspToPc:352 EspRssi:-72 LostFramesPcToEsp:0   0 203 0 0
    Finished Test. Total Lost Ping Pong Cycles: 363

These results are rather displeasing. It was placed at the exact spot where the other board was located before.
The RSSI was even better, but frames sent by the ESP to the PC were lost. So we can assume that the receive direction is not to blame but the Transmit direction is faulty.
Of all 1000 packages 363 have not made the full ping pong cycle.
203 frames had a failing `sendto` so we can be sure that the ESP32 itself was aware of a problem.


## Disclaimer

The author assumes no responsibility or liability for any errors or omissions in the content of this project. The information contained in this site is provided on an "as is" basis with no guarantees of completeness, accuracy, usefulness or timeliness...
