*(AI generated)*
**Project: Long-Distance Girlfriend Memory Box (Simplified Blueprint)**
This blueprint outlines a simplified version of the Memory Box, where a physical on/off switch directly controls the power to the device. This removes the need for a reed switch, deep sleep code, and external pull-up resistors, making the project more straightforward to build.

*Core Features*:
Microcontroller: ESP32-C6 Mini
Display: Blue-Yellow OLED (SSD1306 via I2C)
User Interaction: Up, Down, and OK buttons for navigation.
Power Management: A simple physical on/off switch controls power to the device.
Content Management: Messages and animations are fetched from a GitHub JSON file via Wi-Fi.
_________________________________________________________________________________________


**Phase 1: Component Checklist & Wiring**

    1.1 Component Checklist
ESP32-C6 Mini Development Board
SSD1306 0.96" Blue-Yellow OLED Display
Momentary Push Buttons (3 total: Up, Down, OK)
A physical on/off switch (e.g., a slide switch or a toggle switch).
Your blue pouch LiPo battery and VLYEE-M9003-POWER PCB.
Breadboard, jumper wires, and USB-C cable.
Multimeter.

    1.2 Simplified Hardware Wiring
Power Switch Wiring:

This is the main change. The switch will be placed on the power line to the ESP32.
Connect the positive output (V+) from your VLYEE-M9003-POWER PCB to one terminal of the on/off switch.
Connect the other terminal of the switch to the 5V or 3V3 pin on your ESP32-C6 Mini board (based on your VLYEE board's output voltage).
Connect the negative output (V-) from your VLYEE board directly to a GND pin on your ESP32.
When the switch is off, the ESP32 receives no power and consumes zero current.
OLED Display (SSD1306 I2C) Wiring:
OLED VCC to ESP32 3.3V
OLED GND to ESP32 GND
OLED SDA to ESP32 GPIO 2
OLED SCL to ESP32 GPIO 1
Push Buttons Wiring (3 Buttons Total):
Buttons are still wired to use the ESP32's internal pull-up resistors.
For each button:
Connect one end to ESP32 GND.
Connect the other end to its respective ESP32 GPIO Pin.
"Up" Button: to ESP32 GPIO 3
"Down" Button: to ESP32 GPIO 14
"OK" Button: to ESP32 GPIO 20

***Phase 2: Simplified Arduino Code***
This code removes all deep sleep and reed switch-related functionality. It's streamlined to focus only on button-based message navigation, the menu, and Wi-Fi content fetching.
*code in CODE.c file*
