#include "displayManager.h"

// Define the display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

void initScreen() {
    Wire.begin(SDA_1, SCL_1); // Use GPIO 15 (SDA) and GPIO 16 (SCL)
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;); // Stay in an infinite loop if initialization fails
    }
    // display.display();
    display.clearDisplay();
    // display.setTextSize(1);
    // display.setTextColor(SSD1306_WHITE);
    // display.setCursor(0, 0);
    // display.print("Main Menu");
    // display.display();

}



void displayTwoLinesOfText(String string1, String string2) {
    static unsigned long previousMillisText = 0; 
    const unsigned long intervalText = 1; // limit fastest screen update frequency in milliseconds
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillisText >= intervalText) {
        previousMillisText = currentMillis;

        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE); 

        // Line 1
        display.setCursor(0, 0);
        display.println(string1);

        // Line 2
        display.setCursor(0, 10); 
        display.println(string2);

        display.display();
    }
}

void displayThreeLinesOfText(String string1, String string2 , String string3) {
    static unsigned long previousMillisText = 0; 
    const unsigned long intervalText = 1; // limit fastest screen update frequency in milliseconds
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillisText >= intervalText) {
        previousMillisText = currentMillis;

        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE); 

        // Line 1
        display.setCursor(0, 0);
        display.println(string1);

        // Line 2
        display.setCursor(0, 10); 
        display.println(string2);

        // Line 3
        display.setCursor(0, 20); 
        display.println(string3);

        display.display();
    }
}


void displayLoadingAnimation() {
    static unsigned long previousMillisAnimation = 0; 
    const unsigned long intervalAnimation = 100; 
    static int angle = 0; 
    const int angleIncrement = 30;
    const int radius = 10;
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillisAnimation >= intervalAnimation) {
        previousMillisAnimation = currentMillis;

        display.clearDisplay();
        int16_t centerX = display.width() / 2;
        int16_t centerY = display.height() / 2;

        float radians = angle * (PI / 180);
        int16_t x = centerX + radius * cos(radians);
        int16_t y = centerY + radius * sin(radians);

        display.drawLine(centerX, centerY, x, y, SSD1306_WHITE);
        display.display();

        angle += angleIncrement;
        if (angle >= 360) {
            angle = 0;
        }
    }
}


void resetDisplay() {
    static unsigned long previousMillisText = 0; // Static to retain value
    const unsigned long intervalText = 200; // Interval to display text
    unsigned long currentMillis = millis();

    display.invertDisplay(false);
    if (currentMillis - previousMillisText >= intervalText) {
        previousMillisText = currentMillis;
        display.invertDisplay(true);
    }
}

float measureBattery(int& batteryStatusCheck) {
  int rawValue = analogRead(batteryStatusCheck);
  int upperVoltageLimit = 4.2;
  int lowerVoltageLimit = 3.25;
  float voltage = rawValue * (upperVoltageLimit / 4095.0) * 2; // x2 because voltage dividor
  if (voltage <= lowerVoltageLimit) {
    return 0.0; 
  } else if (voltage >= upperVoltageLimit) {
    return 100.0; 
  } else {
    return ((voltage - lowerVoltageLimit) / (upperVoltageLimit - lowerVoltageLimit)) * 100.0;
      }

}

