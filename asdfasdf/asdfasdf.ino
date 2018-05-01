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
int oldPosition  = -999;
bool selectingNetwork = false;
bool selectingPassword = false;

Encoder myEnc(0, 4);

void setup() {
  Serial.begin(115200);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextWrap(false);

  selectingPassword = true;
  printPasswordSelection();
}

void printNetworks(int startIndex, bool highlight) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Choose a network");
  display.display();
  
  if (startIndex >= 10)
    startIndex = 10 - 1;
    
  for (int i = startIndex; i < startIndex + 3; i++) {
    if (highlight && i == startIndex && startIndex < 10) {
      display.setTextColor(BLACK, WHITE);
    } else {
      display.setTextColor(WHITE);
    }

    if (i < 10) {
      display.println(networks[i]);
    } else {
      display.println("");
    }
    
    display.display();
  }
}

void selectNetwork() {
  ssid = networks[selectedNetworkIndex];
}

void shiftUp() {
  if (selectedNetworkIndex < 9) {
    selectedNetworkIndex += 1;
    printNetworks(selectedNetworkIndex, true);
  }
}

void shiftDown() {
  if (selectedNetworkIndex > 0) {
    selectedNetworkIndex -= 1;
    printNetworks(selectedNetworkIndex, true);
  }
}

void printPasswordSelection() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Enter password!");
  display.println();
  display.println();

  int startingIndex = 0;
  String chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-_+=~`[]{}|\\:;\"'<>,.?/";

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

void shiftRight() {
  if (selectedCharacterIndex < 93) {
    selectedCharacterIndex += 1;
    printPasswordSelection();
  }
}

void shiftLeft() {
  if (selectedCharacterIndex > 0) {
    selectedCharacterIndex -= 1;
    printPasswordSelection();
  }
}

void selectedCharacter() {
}

void deleteCharacter() {
}

void connectToNetwork() {
}

void showNetworkError() {
}

void loop() {
  while (selectedCharacterIndex < 93) {
    shiftRight();
    delay(100);
  }

  while (selectedCharacterIndex > 0) {
    shiftLeft();
    delay(100);
  }
//  if (selectingNetwork) {
//    int newPosition = myEnc.read();
//
//    if (newPosition != oldPosition) {
//      if (newPosition > oldPosition) {
//        shiftUp();
//      } else {
//        shiftDown();
//      }
//
//      oldPosition = newPosition;
//      
//      Serial.println(newPosition);
//    }
//  } else if (selectingPassword) {
//    int newPosition = myEnc.read();
//
//    if (newPosition != oldPosition) {
//      if (newPosition > oldPosition) {
//        shiftRight();
//      } else {
//        shiftLeft();
//      }
//
//      oldPosition = newPosition;
//      
//      Serial.println(newPosition);
//    }
//  } else {
//  }
}

