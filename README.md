# TachoChineRoller
DigitalTachometer for 50cc scooter.

Currently working with mpu6050 and HLRC M100 GPS module. GPS can be just swapped by any other GPS module supporting the standard protocol and supporting 115200bps baud rate (Code can be adjusted for other baud rates)
Using an small 1.24inch aliexpress round lcd screen which gives room for gps speed, gps time, temperature and later engine RPM and other data will be displayed.

Running on ESP32 but should also run fine on every c++ supportive microcontroller (might need to adjust the libraries).
