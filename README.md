# viq_worlds_demo_2015
Demo code for the VEX IQ demo robot at worlds 2015

TI Launchpad is needed to interface the LED matrix to the VEX IQ Brain.

Connection to the VEX IQ brain uses the i2c interface, this is typically
connected to pins P1.6 and P1.7 on an MSP430 device.

This demo only works with the MSP430G2553 due to the memory requirements.
It is similar to the VEX IQ reference sensor but also uses USCIA0 in SPI
mode to communicate with four 8x8 LED matrices from Olimex.

https://www.olimex.com/Products/MSP430/Booster/MOD-LED8x8/open-source-hardware


           MSP430G2553
          +-----------------+
       /|\|              XIN|-
        | |                 |
        --|RST          XOUT|-
          |                 |
          |P2.0         P1.0|-->LED
          |P2.1         P1.1|-->Matrix CS      6P6C offset latch connector
          |P2.2         P1.2|-->Matrix MOSI   +---------------------------+
          |P2.3         P1.3|<--Switch        | 1 DIO (not used)          |
          |P2.4         P1.4|-->Matrix SCK    | 5 7.2V (do not connect)   |
          |P2.5         P1.5|<----------------| 2 I2C_ENABLE              |
          |             P1.6|<----------------| 6 I2C_CLOCK               |
          |             P1.7|<--------------->| 3 I2C_DATA                |
          |                 |              -- | 4 GND                     |
          +-----------------+              |  +---------------------------+
                                          \|/




Two demo programs are provided

matrix_demo.c runs a demo of the LED matrix capability.

simon.c is a simple implementation of the 70's game.

![demo robot]
(https://jpearman.smugmug.com/Robotics/Misc/i-PgGHcvx/0/M/IMG_0367sm-M.jpg)
