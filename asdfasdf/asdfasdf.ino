#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Encoder.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

String networks[] = { "one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten" };
int selectedNetworkIndex = 0;
int selectedCharacterIndex = 0;
String ssid = "";
int oldPosition  = 0;
int lastClickTime = millis();
int lastScrollTime = millis();
int pwdAccel = 1;
bool selectingNetwork = false;
bool selectingPassword = false;
bool readyForClick = true;
String password = "";
const String chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-_+=~`[]{}|\\:;\"'<>,.?/";

Encoder myEnc(0, 4);
const int encoderClick = 13;

void setup() {
  Serial.begin(115200);

  pinMode(encoderClick, INPUT);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextWrap(false);

  selectingPassword = true;
  printPasswordSelection();
}

void printNetworks() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Choose a network");

  for (int i = selectedNetworkIndex; i < selectedNetworkIndex + 3; i++) {
    if (i == selectedNetworkIndex) {
      display.setTextColor(BLACK, WHITE);
    } else {
      display.setTextColor(WHITE);
    }

    if (i < 10) {
      display.println(networks[i]);
    } else {
      display.println("");
    }
  }

  display.display();
}

void selectNetwork() {
  ssid = networks[selectedNetworkIndex];
}

void shiftUp(int amount = 1) {
  if (selectedNetworkIndex + amount > 9) {
    selectedNetworkIndex = 9;
  } else {
    selectedNetworkIndex += amount;
  }

  printNetworks();
}

void shiftDown(int amount = 1) {
  if (selectedNetworkIndex - amount < 0) {
    selectedNetworkIndex = 0;
  } else {
    selectedNetworkIndex -= amount;
  }

  printNetworks();
}

void printPasswordSelection() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Enter password!");

  if (password.length() > 21) {
    display.println(password.substring(password.length() - 21));
  } else {
    display.println(password);
  }

  display.println();

  int startingIndex = 0;

  for (int i = selectedCharacterIndex - 8; i < selectedCharacterIndex; i++) {
    if (i >= 0) {
      display.print(chars.charAt(i));
    } else {
      display.print(" ");
    }
  }

  display.print("  ");
  display.print(chars.charAt(selectedCharacterIndex));
  display.print("  ");

  for (int i = selectedCharacterIndex + 1; i < 94; i++) {
    display.print(chars.charAt(i));
  }

  display.display();
}

void shiftRight(int amount = 1) {
  if (selectedCharacterIndex + amount > 93) {
    selectedCharacterIndex = 93;
  } else {
    selectedCharacterIndex += amount;
  }

  printPasswordSelection();
}

void shiftLeft(int amount = 1) {
  if (selectedCharacterIndex - amount < 0) {
    selectedCharacterIndex = 0;
  } else {
    selectedCharacterIndex -= amount;
  }

  printPasswordSelection();
}

void selectCharacter() {
  password += chars.charAt(selectedCharacterIndex);
  printPasswordSelection();
}

void deleteCharacter() {
}

void connectToNetwork() {
}

void showNetworkError() {
}

void loop() {
  if (selectingNetwork) {
    int newPosition = myEnc.read();

    if (newPosition != oldPosition && newPosition % 4 == 0) {
      if (newPosition > oldPosition) {
        shiftUp(pwdAccel);
      } else {
        shiftDown();
      }

      oldPosition = newPosition;
    }
  } else if (selectingPassword) {
    int newPosition = myEnc.read();

    if (newPosition != oldPosition && newPosition % 4 == 0) {
      if (newPosition > oldPosition) {
        shiftLeft(pwdAccel);
      } else {
        shiftRight(pwdAccel);
      }

      oldPosition = newPosition;

      if (millis() - lastScrollTime < 50) {
        if (pwdAccel < 5)
          pwdAccel += 1;
      }
      
      lastScrollTime = millis();
    } else {
      if (millis() - lastScrollTime > 50)
        pwdAccel = 1;

      if (digitalRead(encoderClick) == LOW && readyForClick && millis() - lastClickTime > 500) {
        lastClickTime = millis();
        readyForClick = false;
        selectCharacter();
      } else if (digitalRead(encoderClick) == HIGH && !readyForClick) {
        readyForClick = true;
      }
    }
  }
}
