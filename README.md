Memory Box: A Long-Distance Love Communication Device
This project details the creation of a "Memory Box" – a custom device designed to help maintain connection over long distances. It features an ESP32-C6 Mini microcontroller, a blue-yellow OLED display, interactive buttons, and a reed switch to detect when the box is opened. Messages and animations are dynamically fetched from a GitHub repository, enabling remote updates and personalized content delivery.
Project Goal
To build a low-power, engaging physical device that displays heartfelt messages and custom animations, delivered remotely via GitHub, offering a unique way to bridge the distance in a relationship.
Core Features
 * Microcontroller: ESP32-C6 Mini (e.g., DFRobot Beetle ESP32-C6)
 * Display: Blue-Yellow OLED (SSD1306 via I2C)
 * User Interaction:
   * Up Button: Navigate messages / scroll text up
   * Down Button: Navigate messages / scroll text down
   * OK Button: Toggle between message viewing and menu mode
 * Box State Detection: Reed switch to detect "box open" status, triggering display activation.
 * Power Management:
   * Deep Sleep mode for ultra-low power consumption when the box is closed.
   * Wake-up triggered by opening the box (reed switch).
 * Content Management:
   * Messages and custom animations stored in a messages.json file on a GitHub repository.
   * ESP32 fetches content from GitHub (via Wi-Fi) at scheduled times (e.g., 6 AM & 6 PM) and on power-up/wake-up.
   * Content is stored on the ESP32's internal flash memory, allowing offline access to the last fetched content.
 * Custom Content: Supports both multi-line text messages (with scrolling capability) and ASCII art animations.
Removed Features (as per project evolution)
 * DFPlayer Mini MP3 Player
 * Audio playback
 * Direct SD card access
Phase 1: Setup Development Environment & Gather Components
1.1 Development Environment Setup
 * Install Visual Studio Code (VS Code): If you don't have it, download and install VS Code from the official website.
 * Install Espressif IDF Extension:
   * Open VS Code.
   * Go to the Extensions view (accessible via the square icon on the sidebar or Ctrl+Shift+X / Cmd+Shift+X).
   * Search for "Espressif IDF" and install the official extension by Espressif Systems.
 * Perform ESP-IDF Express Install:
   * Follow the prompts provided by the Espressif IDF extension. Choose the "Express Install" option. This will automatically download and set up the necessary ESP-IDF framework, toolchain, and other required components.
1.2 Component Checklist
Before you begin wiring, ensure you have all the necessary hardware:
 * Microcontroller: ESP32-C6 Mini Development Board (e.g., DFRobot Beetle ESP32-C6)
 * Display: SSD1306 0.96" Blue-Yellow OLED Display (I2C version)
 * Input:
   * Reed Switch (normally open or normally closed – adapt code if needed for normally closed)
   * Small Magnets (for reed switch activation)
   * Momentary Push Buttons (3 pieces: Up, Down, OK)
 * Resistors: 10k Ohm (1 piece for the reed switch pull-up)
 * Power Solution:
   * Your existing blue pouch LiPo battery
   * Your existing VLYEE-M9003-POWER board (the integrated charging/power management PCB)
 * Connectivity:
   * Breadboard (for prototyping)
   * Jumper Wires (male-to-male, male-to-female)
   * USB-C cable (for flashing and powering the ESP32-C6)
 * Tools (Essential):
   * Multimeter (Absolutely crucial for verifying battery output voltage!)
 * Tools (Optional/For Final Assembly):
   * Project enclosure
   * Soldering iron and solder
Phase 2: Hardware Wiring
Before starting any wiring, ensure the ESP32-C6 Mini board is DISCONNECTED from power. Always double-check your specific ESP32-C6 Mini board's pinout, as GPIO numbers can sometimes differ slightly between manufacturers.
2.1 Power Supply Wiring (using your LiPo & VLYEE-M9003-POWER PCB)
This is the most critical step for proper power management and avoiding damage.
 * Identify Output Voltage:
   * CRUCIAL: Connect your multimeter to the V+ and V- output pads (the ones not directly connected to the battery, but typically towards the top edge) on your VLYEE-M9003-POWER PCB.
   * Measure the voltage.
 * Connect to ESP32-C6 based on Measurement:
   * If output is ~5V:
     * Connect V+ output of VLYEE board to the 5V or VIN pin on your ESP32-C6 Mini board.
     * Connect V- output of VLYEE board to a GND pin on your ESP32.
   * If output is ~3.3V:
     * Connect V+ output of VLYEE board to the 3V3 pin on your ESP32-C6 Mini board.
     * Connect V- output of VLYEE board to a GND pin on your ESP32.
   * If output is ~3.7V-4.2V (direct battery voltage):
     * STOP! This means the VLYEE board might not be regulating the voltage.
     * Check your ESP32-C6 Mini board's documentation carefully. Some ESP32 boards have a dedicated LiPo input (e.g., a JST connector or a specific BAT pin) with built-in charging/protection. If yours does, you can connect the battery directly to that specific input.
     * If your ESP32 board DOES NOT have a dedicated LiPo input with charging/protection, you must use a separate 5V boost converter module between the VLYEE board and the ESP32's 5V/VIN pin. Connecting direct LiPo voltage to a 3.3V-only input can damage the ESP32.
 * Battery Charging:
   * Locate the micro-USB port or specific input pins on your VLYEE-M9003-POWER board (or the original lamp PCB it was part of).
   * This is where you'll connect a standard 5V USB charger to recharge the LiPo battery.
2.2 OLED Display (SSD1306 I2C) Wiring
 * OLED VCC to ESP32 3.3V
 * OLED GND to ESP32 GND
 * OLED SDA to ESP32 GPIO 2 (This corresponds to OLED_SDA_PIN in code)
 * OLED SCL to ESP32 GPIO 1 (This corresponds to OLED_SCL_PIN in code)
2.3 Reed Switch Wiring
 * Reed Switch (one lead) to ESP32 GND
 * Reed Switch (other lead) to ESP32 GPIO 0
 * External Pull-up Resistor: Connect a 10k Ohm resistor from ESP32 GPIO 0 (the same pin connected to the reed switch) to ESP32 3.3V.
   * Logic: When the magnet is away, the switch is open, and GPIO 0 is pulled HIGH (3.3V) by the resistor. When the magnet is close, the switch closes, connecting GPIO 0 to GND, making the pin LOW. This pin will be configured as an EXT1 (or EXT0) wakeup source for deep sleep.
2.4 Push Buttons Wiring (3 Buttons)
For each button, connect one end to GND and the other end to the specified ESP32 GPIO pin. We'll use the ESP32's internal pull-up resistors in software, so no external resistors are needed here.
 * "Up" Button:
   * One end to ESP32 GND
   * Other end to ESP32 GPIO 3 (BUTTON_UP_PIN in code)
 * "Down" Button:
   * One end to ESP32 GND
   * Other end to ESP32 GPIO 14 (BUTTON_DOWN_PIN in code)
 * "OK" Button:
   * One end to ESP32 GND
   * Other end to ESP32 GPIO 20 (BUTTON_OK_PIN in code)
Phase 3: Software Development (in VS Code / ESP-IDF)
This phase involves configuring your ESP-IDF project, including the necessary libraries, and writing the core application logic.
3.1 Create a New ESP-IDF Project
 * Open VS Code.
 * Open the Command Palette: Press Ctrl+Shift+P (Windows/Linux) or Cmd+Shift+P (macOS).
 * Select "ESP-IDF: New Project": Choose this option from the dropdown.
 * Choose "Start from scratch": This provides a minimal project template.
 * Name Your Project: Provide a meaningful name (e.g., memory_box).
 * Select Your Target: Choose esp32c6 as your target device.
 * Select Project Folder: Choose a directory on your computer where you want to save your project.
 * VS Code will then generate the basic project structure with main component and CMakeLists.txt files.
3.2 Add External Libraries and Configure Project
 * Adafruit SSD1306 and GFX Libraries:
   * These are typically installed through the Arduino IDE's Library Manager. For ESP-IDF, you'll generally download them and place them in your project's components directory.
   * Method: The easiest way is often to use them via the Arduino as an ESP-IDF component. Your current setup already uses the Arduino framework for ESP32.
   * Ensure your main/CMakeLists.txt (or equivalent) points to the necessary Arduino components and any specific Adafruit SSD1306/GFX libraries if they are not part of the standard ESP-IDF setup.
   * Self-correction: Given the previous code examples used Arduino-style setup() and loop(), it's clear this project is built on the Arduino framework for ESP32, not pure ESP-IDF. While VS Code with the Espressif IDF Extension can manage Arduino projects, the project setup process mentioned "Start from scratch" which is more typical for pure ESP-IDF.
   Let's clarify for the README based on the code you're using:
   Refined 3.1 & 3.2 for Arduino IDE / VS Code with Arduino Extension
   Since your code uses setup() and loop(), it means you're using the Arduino Framework on ESP32.
   * Install Arduino IDE / Use VS Code with PlatformIO Extension:
     * If using Arduino IDE: Download and install it. Go to File > Preferences > Additional Boards Manager URLs and add https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json. Then go to Tools > Board > Boards Manager and search for "esp32" to install the "esp32 by Espressif Systems" package.
     * If using VS Code: Install the PlatformIO extension. Create a new PlatformIO project, select your ESP32-C6 board, and it will set up the Arduino framework automatically.
   * Install Libraries:
     * Open your Arduino IDE (or PlatformIO in VS Code).
     * Go to Sketch > Include Library > Manage Libraries... (or PlatformIO equivalent).
     * Search for Adafruit SSD1306 and install it.
     * Search for Adafruit GFX Library and install it (SSD1306 depends on this).
     * Search for ArduinoJson and install the latest version (v6 or higher) by Benoît Blanchon.
3.3 Main Application Logic (.ino or .cpp file)
Your main code file (e.g., MemoryBox.ino or main.cpp if using PlatformIO) will contain the following high-level sections and logic:
 * Includes: Include necessary libraries (WiFi.h, HTTPClient.h, ArduinoJson.h, Adafruit_SSD1306.h, Wire.h, esp_sleep.h).
 * Configuration:
   * WiFi SSID and Password.
   * GitHub raw JSON URL (jsonUrl).
   * OLED display pin definitions (OLED_SDA_PIN, OLED_SCL_PIN, OLED_RESET).
   * Button pin definitions (BUTTON_UP_PIN, BUTTON_DOWN_PIN, BUTTON_OK_PIN).
   * Reed switch pin definition (REED_SWITCH_PIN).
   * JSON_DOC_SIZE constant (Crucial: update this using ArduinoJson Assistant after any JSON content changes).
 * Global Variables:
   * Adafruit_SSD1306 display; object.
   * Variables for button debouncing (lastButtonPressTime, debounceDelay).
   * StaticJsonDocument<JSON_DOC_SIZE> doc; and JsonArray messagesArray; for JSON parsing.
   * Message tracking variables (currentMessageIndex, totalMessages, wifiConnected, hasDownloadedMessages).
   * Scrolling variables for text messages (firstVisibleLineIndex, LINES_PER_BLUE_SCREEN, currentMessageLines[], currentMessageTotalLines).
   * Animation variables for menu and JSON animations (currentAnimationFrameIndex, lastAnimationFrameTime, menuAnimFrames[], currentMenuAnimFrame, lastMenuAnimTime, MENU_ANIM_DELAY).
   * UI mode (UI_Mode currentMode).
 * setup() Function:
   * Initialize OLED display (display.begin).
   * Configure button pins with INPUT_PULLUP.
   * Configure reed switch pin as INPUT.
   * Set up deep sleep wakeup source (esp_sleep_enable_ext1_wakeup) for the reed switch.
   * Connect to Wi-Fi.
   * Synchronize time via NTP (configTime).
   * Call fetchAndParseJson() to download messages.
   * Initialize currentMessageIndex and call resetMessageDisplayState().
   * Initial call to displayYellowHeader().
 * loop() Function:
   * Continuously check the reed switch state.
   * If the box is closed (reed switch LOW), call goToDeepSleep().
   * If the box is open (reed switch HIGH), call handleButtonPresses() and displayYellowHeader().
   * Include a small delay() for stability and power saving.
 * Helper Functions:
   * printToOLED(...): Clears display, sets cursor/text, prints, and updates display.
   * displayYellowHeader(): Draws the top "yellow" section with time, dynamic message, and WiFi status. Calls displayBlueContent() or showMenuUI().
   * displayBlueContent():
     * Determines if the current message is text or animation type.
     * For text messages: Manages scrolling logic using firstVisibleLineIndex and currentMessageLines.
     * For animation messages: Iterates through frames based on display_time_seconds and displays the current frame, centering it.
   * loadMessageIntoLines(const char* messageContent): Parses a multi-line string into an array of Strings for scrolling.
   * fetchAndParseJson(): Connects to GitHub, downloads the messages.json, and parses it using ArduinoJson. Handles network and parsing errors.
   * handleButtonPresses():
     * Debounces button presses.
     * Manages message navigation (previous/next message) and text scrolling (up/down).
     * Switches currentMode between MESSAGE_MODE and MENU_MODE via the OK button.
     * Calls resetMessageDisplayState() when the message index changes or returning from the menu.
   * goToDeepSleep(): Prepares the ESP32 for deep sleep and initiates it.
   * showMenuUI():
     * Displays "MENU" text.
     * Plays the custom menu animation by splitting its multi-line frames and centering each line within the remaining screen space.
   * resetMessageDisplayState(): Resets scroll indices and animation frame counters when a new message is selected. Loads lines for new text messages and clears lines for non-text messages.
Phase 4: Content Creation & Upload
4.1 messages.json Structure
Your GitHub repository should contain a messages.json file with the following structure:
{
  "messages": [
    {
      "id": 1,
      "type": "text",
      "display_time_seconds": 30,
      "content": "Your multi-line text message goes here.\nUse \\n for new lines.\nThis can be quite long and scrollable."
    },
    {
      "id": 2,
      "type": "animation",
      "display_time_seconds": 0.3, // Duration for EACH frame
      "frames": [
        "Frame 1 ASCII art",
        "Frame 2 ASCII art",
        "Frame 3 ASCII art"
      ]
    },
    {
      "id": 3,
      "type": "text",
      "display_time_seconds": 15,
      "content": "Another shorter message.\nThis is a beautiful message for your girlfriend!\nIt will scroll if it has more than 4 lines.\nThis is line 4.\nThis is line 5."
    }
    // ... add more messages as desired
  ]
}

 * id: Unique identifier for the message.
 * type: Can be "text" or "animation".
 * display_time_seconds:
   * For text messages: How long the message is displayed before automatically moving to the next (if auto-advance were implemented, but currently controlled by user buttons). For scrolling text, this is more of a placeholder or hint for future features.
   * For animation messages: The duration (in seconds) that each individual frame is displayed.
 * content (for type: "text"): A single string containing your text. Use \n to indicate line breaks.
 * frames (for type: "animation"): An array of strings, where each string is one frame of your ASCII art animation. Ensure each frame string is on a single logical line in the JSON file.
4.2 Upload to GitHub
 * Create a new public GitHub repository (e.g., MemoryBoxContent).
 * Upload your messages.json file to the root of this repository.
 * Ensure the file is publicly accessible.
 * Get the raw URL for your messages.json file. This is the URL you'll use in your ESP32 code (e.g., https://raw.githubusercontent.com/YourUsername/YourRepoName/main/messages.json).
Phase 5: Final Assembly & Deployment
 * Test thoroughly on a breadboard before making any permanent connections.
 * Verify all wiring for correctness.
 * Integrate components into your chosen project enclosure.
 * Secure the reed switch and magnet in appropriate positions so that the reed switch closes (or opens, depending on your setup) when the box is closed.
 * Charge the LiPo battery.
 * Deploy your Memory Box!
This blueprint provides a comprehensive guide for building your Memory Box. Good luck with the project!
