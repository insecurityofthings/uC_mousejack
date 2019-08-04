Arduino Mousejack (ShutDô) ![projects-microcontroller](https://img.shields.io/badge/projects-microcontroller-blue)
-----------------

Arduino Mousejack is a project, based on **uC_mousejeck** by **[insecuritythings](https://github.com/insecurityofthings/uC_mousejack)**, to get [Mousejack](https://www.mousejack.com) attacks into a small embedded device, with the form factor of a key chain.

![Prototype Mousejack Device](https://store.arduino.cc/usa/arduino-uno-rev3:small)

Building the device is straight-forward, and the code provides a tool to use Duckyscript to launch automated keystroke injection attacks against Microsoft and Logitech devices.

Construction
------------

Components list:
 - [Arduino UNO](https://www.adafruit.com/products/2771) or another Arduino-compatible board of your choice.
 - [NRF24L01+ module](http://www.icstation.com/22dbm-100mw-nrf24l01ppalna-wireless-transmission-module-p-4677.html) or another compatible device.
 - A 10μF capacitor to help stabilize the voltage for the NRF24 module.
 - Wires to connections.

 The Fritzing diagram below shows the wiring layout used in the prototype design:

 ![Mousejack Fritzing Design](https://raw.githubusercontent.com/dnatividade/Arduino_mousejack/master/img/Arduino-MouseJack2_bb.png)

 Building
 --------

 To build the software, download and install the [PlatformIO IDE](http://platformio.org/platformio-ide).

 Before building the software, be sure to modify the `attack.h` file using the `attack_generator.py` script:

 ```
 Arduino-mousejack $ cd tools
 tools $ ./attack_generator.py ducky.txt
 ```

 In the example above, the ducky.txt file contains our [Duckyscript](https://github.com/hak5darren/USB-Rubber-Ducky/wiki/Duckyscript). The `attack_generator.py` script will "compile" the ducky script into the `attack.h` file, which is included in `main.cpp`. This simplifies the code and makes it more compact.

 Testing
 -------

 Once you power the device on, the internal LED connected to pin 13 (called ledpin in the code), will blink two times for each pass over the entire channel range. When it sends an attack, the LED will glow solid.

 If you monitor the serial port using the PlatformIO IDE, you will see the radio information details and a lot of debugging information being printed while scanning and during attack.

 Warning: No interaction is required to initiate an attack.
 
  **More information about the tests performed, as well as the difficulties encountered, can be found in the [testing](./testing/) directory.**

 Future
 ------

 As future work:
 - to create a python program to interact with arduino, passing parameters to the runtime attack;
 - to test the same project with Arduino NanoRF.
  
