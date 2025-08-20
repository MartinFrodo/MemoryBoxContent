#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

// --- WiFi Configuration (UPDATE THESE!) ---
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// --- GitHub JSON URL (Verify this URL is correct and raw) ---
const char* jsonUrl = "https://raw.githubusercontent.com/MartinFrodo/MemoryBoxContent/main/messages.json";

// --- OLED Display Configuration ---
#define OLED_SDA_PIN 2
#define OLED_SCL_PIN 1
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- Button Pin Definitions ---
const int BUTTON_UP_PIN = 3;
const int BUTTON_DOWN_PIN = 14;
const int BUTTON_OK_PIN = 20;

// --- Button State Variables for Debounce ---
long lastButtonPressTime = 0;
const long debounceDelay = 150; // Milliseconds

// --- JSON Data Storage ---
// IMPORTANT: Update JSON_DOC_SIZE using ArduinoJson Assistant
const int JSON_DOC_SIZE = 512;
StaticJsonDocument<JSON_DOC_SIZE> doc;
JsonArray messagesArray;

// --- Message Display Variables ---
int currentMessageIndex = 0;
int totalMessages = 0;
bool wifiConnected = false;
bool hasDownloadedMessages = false;

// --- Scrolling Variables (for text messages) ---
int firstVisibleLineIndex = 0;
const int LINES_PER_BLUE_SCREEN = 4;
String currentMessageLines[50];
int currentMessageTotalLines = 0;

// --- Animation Variables (for JSON animation messages) ---
int currentAnimationFrameIndex = 0;
unsigned long lastAnimationFrameTime = 0;

// --- State Machine for UI Modes ---
enum UI_Mode {
    MESSAGE_MODE,
    MENU_MODE
};
UI_Mode currentMode = MESSAGE_MODE;

// --- Forward Declarations ---
void printToOLED(const char* message, int cursorX, int cursorY, int textColor, int textSize);
void displayYellowHeader();
void displayBlueContent();
void fetchAndParseJson();
void handleButtonPresses();
void showMenuUI();
void loadMessageIntoLines(const char* messageContent);
void resetMessageDisplayState();

// --- SETUP ---
void setup() {
    Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        while (true) { delay(100); }
    }
    printToOLED("OLED Init OK!", 0, 0, SSD1306_WHITE, 1);
    delay(1000);

    // Configure Buttons with Internal Pull-ups
    pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
    pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
    pinMode(BUTTON_OK_PIN, INPUT_PULLUP);

    printToOLED("Hardware Configured!", 0, 0, SSD1306_WHITE, 1);
    delay(1000);

    // --- Connect to WiFi and Fetch JSON ---
    printToOLED("Connecting WiFi...", 0, 0, SSD1306_WHITE, 1);
    WiFi.begin(ssid, password);
    int attempts = 0;
    const int maxWifiAttempts = 20;
    while (WiFi.status() != WL_CONNECTED && attempts < maxWifiAttempts) {
        delay(500);
        display.clearDisplay();
        display.setCursor(0,0);
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.print(("Connecting " + String(attempts + 1) + "/" + String(maxWifiAttempts) + "...").c_str());
        display.display();
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        printToOLED("WiFi Connected!", 0, 0, SSD1306_WHITE, 1);
        delay(1000);
        printToOLED(WiFi.localIP().toString().c_str(), 0, 0, SSD1306_WHITE, 1);
        delay(2000);
        configTime(0, 0, "pool.ntp.org"); // this is time for central europe reall good is working[you should have this in code]configTime(3600, 3600, "pool.ntp.org", "CET-1CEST,M3.5.0,M10.5.0/3");//
        printToOLED("Time Synced!", 0, 0, SSD1306_WHITE, 1);
        delay(1000);
        fetchAndParseJson();
    } else {
        wifiConnected = false;
        printToOLED("WiFi FAILED!", 0, 0, SSD1306_WHITE, 1);
        delay(2000);
        if (!hasDownloadedMessages) {
             printToOLED("No msgs loaded yet!", 0, 0, SSD1306_WHITE, 1);
             delay(2000);
        } else {
            printToOLED("Using old msgs!", 0, 0, SSD1306_WHITE, 1);
            delay(2000);
        }
    }

    printToOLED("Setup Complete!", 0, 0, SSD1306_WHITE, 1);
    delay(1000);

    if (totalMessages > 0) {
        currentMessageIndex = totalMessages - 1;
        resetMessageDisplayState();
    } else {
        currentMessageIndex = 0;
    }
    displayYellowHeader();
}

// --- LOOP ---
void loop() {
    handleButtonPresses();
    displayYellowHeader();
    delay(50);
}

// --- UTILITY FUNCTIONS ---

// Function to print text to specific OLED coordinates with color and size
void printToOLED(const char* message, int cursorX, int cursorY, int textColor, int textSize) {
    display.clearDisplay();
    display.setTextSize(textSize);
    display.setTextColor(textColor);
    display.setCursor(cursorX, cursorY);
    display.print(message);
    display.display();
}

void resetMessageDisplayState() {
    firstVisibleLineIndex = 0;
    currentMessageTotalLines = 0;
    currentAnimationFrameIndex = 0;
    lastAnimationFrameTime = 0;
    for (int i = 0; i < 50; i++) {
        currentMessageLines[i] = "";
    }
    if (totalMessages > 0) {
        JsonObject currentMsg = messagesArray[currentMessageIndex];
        if (!currentMsg.isNull()) {
            if (strcmp(currentMsg["type"], "text") == 0) {
                loadMessageIntoLines(currentMsg["content"]);
            }
        }
    }
}

void displayYellowHeader() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    struct tm timeinfo;
    time_t now = time(nullptr);
    char timeStr[6];
    String headerMsg;

    if (!getLocalTime(&timeinfo)) {
        headerMsg = "Time not set!";
    } else {
        strftime(timeStr, sizeof(timeStr), "%H:%M", &timeinfo);
        int hour = timeinfo.tm_hour;
        if (hour >= 5 && hour < 12) {
            headerMsg = "Good Morning! " + String(timeStr);
        } else if (hour >= 12 && hour < 18) {
            headerMsg = "Hello! " + String(timeStr);
        } else if (hour >= 18 && hour < 22) {
            headerMsg = "Good Evening! " + String(timeStr);
        } else {
            headerMsg = "Good Night! " + String(timeStr);
        }
    }
    String wifiIcon = wifiConnected ? " ^" : " X";
    headerMsg += wifiIcon;
    display.setCursor(0, 0);
    display.print(headerMsg);
    
    if (currentMode == MESSAGE_MODE) {
        displayBlueContent();
    } else if (currentMode == MENU_MODE) {
        showMenuUI();
    }
    display.display();
}

void loadMessageIntoLines(const char* messageContent) {
    currentMessageTotalLines = 0;
    String contentStr = String(messageContent);
    int startIndex = 0;
    int endIndex = -1;
    for (int i = 0; i < 50; i++) currentMessageLines[i] = "";
    while (startIndex < contentStr.length()) {
        endIndex = contentStr.indexOf('\n', startIndex);
        if (endIndex == -1) {
            currentMessageLines[currentMessageTotalLines++] = contentStr.substring(startIndex);
            break;
        } else {
            currentMessageLines[currentMessageTotalLines++] = contentStr.substring(startIndex, endIndex);
            startIndex = endIndex + 1;
        }
        if (currentMessageTotalLines >= 50) {
             break;
        }
    }
    firstVisibleLineIndex = 0;
}

void displayBlueContent() {
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    if (totalMessages == 0 || messagesArray.isNull()) {
        display.setCursor(0, 16);
        display.print("No messages loaded.");
        return;
    }
    JsonObject message = messagesArray[currentMessageIndex];
    if (message.isNull()) {
        display.setCursor(0, 16);
        display.print("Error: Invalid msg data.");
        return;
    }
    const char* type = message["type"];
    int currentY = 16;
    if (strcmp(type, "text") == 0) {
        if (currentMessageTotalLines == 0) {
            const char* content = message["content"];
            if (content) {
                loadMessageIntoLines(content);
            } else {
                display.setCursor(0, 16);
                display.print("Text content missing!");
                return;
            }
        }
        for (int i = 0; i < LINES_PER_BLUE_SCREEN; i++) {
            int lineToShowIndex = firstVisibleLineIndex + i;
            if (lineToShowIndex < currentMessageTotalLines) {
                display.setCursor(0, currentY + (i * 10));
                display.print(currentMessageLines[lineToShowIndex]);
            } else {
                display.setCursor(0, currentY + (i * 10));
                display.print("");
            }
        }
    } else if (strcmp(type, "animation") == 0) {
        JsonArray frames = message["frames"].as<JsonArray>();
        float frameDurationFloat = message["display_time_seconds"].as<float>();
        unsigned long frameDurationMs = (unsigned long)(frameDurationFloat * 1000);
        if (frames && frames.size() > 0) {
            if (millis() - lastAnimationFrameTime > frameDurationMs) {
                currentAnimationFrameIndex = (currentAnimationFrameIndex + 1) % frames.size();
                lastAnimationFrameTime = millis();
            }
            const char* currentFrame = frames[currentAnimationFrameIndex];
            if (currentFrame) {
                int16_t x1, y1;
                uint16_t w, h;
                display.setTextSize(1);
                display.getTextBounds(currentFrame, 0, 0, &x1, &y1, &w, &h);
                int centerX = (SCREEN_WIDTH - w) / 2;
                int centerY = 16 + (48 - h) / 2;
                display.setCursor(centerX, centerY);
                display.print(currentFrame);
            }
        } else {
            display.setCursor(0, 16);
            display.print("Anim frames missing!");
        }
    } else {
        display.setCursor(0, 16);
        display.print("Unknown msg type!");
    }
}

void fetchAndParseJson() {
    HTTPClient http;
    http.begin(jsonUrl);
    int httpCode = http.GET();
    if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            printToOLED("JSON Rcvd!", 0, 0, SSD1306_WHITE, 1);
            delay(1000);
            DeserializationError error = deserializeJson(doc, payload);
            if (error) {
                printToOLED("JSON Parse FAILED!", 0, 0, SSD1306_WHITE, 1);
                String errorMsg = "Parse failed: ";
                errorMsg += error.c_str();
                printToOLED(errorMsg.c_str(), 0, 16, SSD1306_WHITE, 1);
                delay(3000);
            } else {
                messagesArray = doc["messages"].as<JsonArray>();
                if (messagesArray) {
                    totalMessages = messagesArray.size();
                    printToOLED("Messages Loaded!", 0, 0, SSD1306_WHITE, 1);
                    delay(1000);
                    hasDownloadedMessages = true;
                } else {
                    printToOLED("No 'messages' key!", 0, 0, SSD1306_WHITE, 1);
                    delay(2000);
                }
            }
        } else {
            String errorMsg = "HTTP Error: ";
            errorMsg += String(httpCode) + " - " + http.errorToString(httpCode).c_str();
            printToOLED(errorMsg.c_str(), 0, 0, SSD1306_WHITE, 1);
            delay(3000);
        }
    } else {
        printToOLED("HTTP Conn Error!", 0, 0, SSD1306_WHITE, 1);
        delay(3000);
    }
    http.end();
}

void handleButtonPresses() {
    int upButtonState = digitalRead(BUTTON_UP_PIN);
    int downButtonState = digitalRead(BUTTON_DOWN_PIN);
    int okButtonState = digitalRead(BUTTON_OK_PIN);
    long currentTime = millis();
    if (currentTime - lastButtonPressTime > debounceDelay) {
        bool messageChanged = false;
        if (upButtonState == LOW) {
            if (currentMode == MESSAGE_MODE) {
                if (totalMessages > 0 && strcmp(messagesArray[currentMessageIndex]["type"], "text") == 0) {
                    if (firstVisibleLineIndex > 0) {
                        firstVisibleLineIndex--;
                    } else {
                        firstVisibleLineIndex = max(0, currentMessageTotalLines - LINES_PER_BLUE_SCREEN);
                    }
                } else {
                    currentMessageIndex--;
                    if (currentMessageIndex < 0) {
                        currentMessageIndex = totalMessages - 1;
                    }
                    messageChanged = true;
                }
            }
            lastButtonPressTime = currentTime;
        } else if (downButtonState == LOW) {
            if (currentMode == MESSAGE_MODE) {
                if (totalMessages > 0 && strcmp(messagesArray[currentMessageIndex]["type"], "text") == 0) {
                    if (firstVisibleLineIndex < currentMessageTotalLines - LINES_PER_BLUE_SCREEN) {
                        firstVisibleLineIndex++;
                    } else {
                        firstVisibleLineIndex = 0;
                    }
                } else {
                    currentMessageIndex++;
                    if (currentMessageIndex >= totalMessages) {
                        currentMessageIndex = 0;
                    }
                    messageChanged = true;
                }
            }
            lastButtonPressTime = currentTime;
        } else if (okButtonState == LOW) {
            if (currentMode == MESSAGE_MODE) {
                currentMode = MENU_MODE;
            } else {
                currentMode = MESSAGE_MODE;
                currentMessageIndex = totalMessages > 0 ? totalMessages - 1 : 0;
                messageChanged = true;
            }
            lastButtonPressTime = currentTime;
            displayYellowHeader();
        }
        if (messageChanged) {
            resetMessageDisplayState();
        }
    }
}

// --- Menu UI Function ---
int currentMenuAnimFrame = 0;
const char* menuAnimFrames[] = {
    "   .-'-.   \n"
    "  ( o_o )  \n"
    "   \\_^_/   \n"
    "           \n"
    "           ",
    "   .-'-.   \n"
    "  (-_-)   \n"
    "   \\_^_/   \n"
    "           \n"
    "           ",
    "   .-'-.   \n"
    "  ( ^_^ )  \n"
    "   \\_^_/   \n"
    "           \n"
    "           ",
    "           \n"
    "           \n"
    "           \n"
    "           \n"
    "           "
};
const int MENU_ANIM_FRAME_COUNT = sizeof(menuAnimFrames) / sizeof(menuAnimFrames[0]);
unsigned long lastMenuAnimTime = 0;
const unsigned long MENU_ANIM_DELAY = 300;

void showMenuUI() {
    const char* menuText = "MENU";
    display.setTextSize(1);
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(menuText, 0, 0, &x1, &y1, &w, &h);
    int menuX = (SCREEN_WIDTH - w) / 2;
    int menuY = 16;
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(menuX, menuY);
    display.print(menuText);
    const char* currentAnimFrameContent = menuAnimFrames[currentMenuAnimFrame];
    String frameStr = String(currentAnimFrameContent);
    int animLineIndex = 0;
    int animStartIndex = 0;
    int animEndIndex = -1;
    String animLines[6];
    while (animStartIndex < frameStr.length() && animLineIndex < 6) {
        animEndIndex = frameStr.indexOf('\n', animStartIndex);
        if (animEndIndex == -1) {
            animLines[animLineIndex++] = frameStr.substring(animStartIndex);
            break;
        } else {
            animLines[animLineIndex++] = frameStr.substring(animStartIndex, animEndIndex);
            animStartIndex = animEndIndex + 1;
        }
    }
    int totalAnimLines = animLineIndex;
    int animPixelHeight = totalAnimLines * 8;
    int animStartingY = menuY + h + (40 - animPixelHeight) / 2;
    if (millis() - lastMenuAnimTime > MENU_ANIM_DELAY) {
        currentMenuAnimFrame = (currentMenuAnimFrame + 1) % MENU_ANIM_FRAME_COUNT;
        lastMenuAnimTime = millis();
    }
    display.setTextSize(1);
    for (int i = 0; i < totalAnimLines; i++) {
        const char* line = animLines[i].c_str();
        display.getTextBounds(line, 0, 0, &x1, &y1, &w, &h);
        int lineX = (SCREEN_WIDTH - w) / 2;
        display.setCursor(lineX, animStartingY + (i * 8));
        display.print(line);
    }
}
