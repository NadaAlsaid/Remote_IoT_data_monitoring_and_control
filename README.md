<h1 align = "center">Remote IoT data monitoring and control using WiFi</h1>
<h2>Project description</h2>
In this project, the Arduino board is connected to Wi-Fi module and both act as a server. The Arduino Board is connected to 1 LED , 3 sensors (temperature, smoke, motion). The client which is a web browser - running on any computer connected to the same network as the Arduino- communicates to the server and controls it.

The project does the following 3 processes:</br>
- The Arduino board monitors and displays sensors data periodically (Temp, Motion, smoke) on LCD every 2 seconds ( using timer1 , using external interrupts (INT0) to detect and read the motion sensor , using ADC functions for reading an analogue value from the sensors ).</br>
- The webpage will send a command to the Wi-Fi module to control the Arduino board and switch each of the LEDs on/off.</br>
- The Arduino board detect abnormal conditions and report a notification alert to the web page. The abnormal conditions are:
  1. Tempertesure value exceeding 30 degrees.
  2. Detecting an existence of smoke.

<h2>Components :</h2>

- Arduino UNO
- Wi-Fi Serial TTL Module ESP-01S 
- LCD
- Temperature sensor: LM35
- Smoke sensor MQ2 
- IR sensor module 
- LED

# Collaborators:
- <a href="https://github.com/NadaAlsaid">Nada Alsaid</a><br>
- <a href="https://github.com/anna-adel">Yoana Adel</a><br>
- <a href="https://github.com/maHossam9">Mariam Hossam</a><br>
- Zeyad Atef
- Abdullah Waleed

