#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Encoder.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

String networks[50];
int selectedNetworkIndex = 0;
int selectedCharacterIndex = 0;
String ssid = "";
String password = "";
int oldPosition  = 0;
int lastClickTime = millis();
int lastScrollTime = millis();
int scroll = 1;
bool selectingNetwork = false;
bool selectingPassword = false;
bool readyToBroadcast = false;
bool broadcasting = false;
const String chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-_+=~`[]{}|\\:;\"'<>,.?/";
bool readyToClick = true;
bool readyToPress = true;

Encoder myEnc(0, 4);
const int encoderClick = 13;

void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);

  pinMode(encoderClick, INPUT);
  pinMode(5, OUTPUT);
  pinMode(12, INPUT);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  delay(100);

  display.clearDisplay();
  display.setTextWrap(false);

  resetDisplay();

  display.println("hello.");

  display.display();

  delay(2000);

  if (readConnection()) {
    connectToNetwork();
  } else {
    clearConnection();
    resetDisplay();
    display.println();
    display.println("scanning networks...");
    display.display();
    scanNetworks();
  }
}

void scanNetworks() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  int n = WiFi.scanNetworks();

  if (n == 0) {
    resetDisplay();

    display.println("No networks found.");
  } else {
    int len = (n > 50) ? 50 : n;

    for (int i = 0; i < len; i++) {
      networks[i] = WiFi.SSID(i);
    }

    showNetworks();
  }
}

bool clearConnection() {
  for (int i = 0; i < 512; i++) {
    EEPROM.write(i, '\0');
  }
}

bool readConnection() {
  char check[3];
  check[2] = '\0';
  check[0] = char(EEPROM.read(0));
  check[1] = char(EEPROM.read(1));

  if (strcmp(check, ":)") != 0)
    return false;

  ssid = "";
  password = "";

  int index = 2;
  char currentChar = char(EEPROM.read(index));

  while (currentChar != '\0') {
    ssid += currentChar;
    index++;
    currentChar = char(EEPROM.read(index));
  }

  Serial.println(ssid);

  index++;
  currentChar = char(EEPROM.read(index));

  while (currentChar != '\0') {
    password += currentChar;
    index++;
    currentChar = char(EEPROM.read(index));
  }

  Serial.println(password);

  return ssid.length() > 0 && password.length() > 0;
}

bool writeConnection() {
  int index = 2;

  EEPROM.write(0, ':');
  EEPROM.write(1, ')');

  for (int i = 0; i < ssid.length(); i++) {
    EEPROM.write(index, ssid.charAt(i));
    index++;
  }

  EEPROM.write(index, '\0');
  index++;

  for (int i = 0; i < password.length(); i++) {
    EEPROM.write(index, password.charAt(i));
    index++;
  }

  EEPROM.write(index, '\0');

  return EEPROM.commit();
}

void resetDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
}

void showNetworks() {
  selectingPassword = false;
  selectingNetwork = true;
  readyToBroadcast = false;
  broadcasting = false;

  resetDisplay();

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

void shiftUp(int amount = 1) {
  if (selectedNetworkIndex + amount > 9) {
    selectedNetworkIndex = 9;
  } else {
    selectedNetworkIndex += amount;
  }

  showNetworks();
}

void shiftDown(int amount = 1) {
  if (selectedNetworkIndex - amount < 0) {
    selectedNetworkIndex = 0;
  } else {
    selectedNetworkIndex -= amount;
  }

  showNetworks();
}

void showPasswordSelection() {
  selectingPassword = true;
  selectingNetwork = false;
  readyToBroadcast = false;
  broadcasting = false;

  resetDisplay();

  display.println("Enter password");

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

  showPasswordSelection();
}

void shiftLeft(int amount = 1) {
  if (selectedCharacterIndex - amount < 0) {
    selectedCharacterIndex = 0;
  } else {
    selectedCharacterIndex -= amount;
  }

  showPasswordSelection();
}

void selectCharacter() {
  password += chars.charAt(selectedCharacterIndex);
  showPasswordSelection();
}

void selectNetwork() {
  ssid = networks[selectedNetworkIndex];

  resetDisplay();

  display.println(ssid);

  display.display();

  delay(1000);

  showPasswordSelection();
}

void showError(String error) {

}

void connectToNetwork() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  resetDisplay();

  display.println("connecting...");
  display.display();

  WiFi.begin(ssid.c_str(), password.c_str());

  int attempts = 0;

  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    display.print(".");
    display.display();
    attempts++;
  }

  if (WiFi.status() != WL_CONNECTED) {
    resetDisplay();
    display.println("failed to connect.");
    display.display();
    delay(1000);
    ssid = "";
    password = "";
    scanNetworks();
  } else {
    if (writeConnection()) {
      showReadyToBroadcast();
    } else {
      showError(":(");
    }
  }
}

void showReadyToBroadcast() {
  selectingPassword = false;
  selectingNetwork = false;
  readyToBroadcast = true;
  broadcasting = false;

  resetDisplay();

  display.println("READY");
  display.display();
}

void loop() {
  if (selectingNetwork) {
    int newPosition = myEnc.read();

    if (newPosition != oldPosition && newPosition % 4 == 0) {
      if (newPosition > oldPosition) {
        shiftUp(scroll);
      } else {
        shiftDown(scroll);
      }

      oldPosition = newPosition;
    } else {
      if (digitalRead(encoderClick) == LOW && readyToClick && millis() - lastClickTime > 500) {
        readyToClick = false;
        lastClickTime = millis();
        selectNetwork();
      } else if (digitalRead(encoderClick) == HIGH && !readyToClick) {
        readyToClick = true;
      }
    }
  } else if (selectingPassword) {
    int newPosition = myEnc.read();

    if (newPosition != oldPosition && newPosition % 4 == 0) {
      if (newPosition > oldPosition) {
        shiftLeft(scroll);
      } else {
        shiftRight(scroll);
      }

      oldPosition = newPosition;

      if (millis() - lastScrollTime < 100) {
        if (scroll < 5)
          scroll += 1;
      }

      lastScrollTime = millis();
    } else {
      if (millis() - lastScrollTime > 100)
        scroll = 1;

      if (digitalRead(12) == HIGH && readyToPress) {
        readyToPress = false;
        connectToNetwork();
      } else if (digitalRead(12) == LOW && !readyToPress) {
        readyToPress = true;
      } else if (digitalRead(encoderClick) == LOW && readyToClick && millis() - lastClickTime > 500) {
        readyToClick = false;
        lastClickTime = millis();
        selectCharacter();
      } else if (digitalRead(encoderClick) == HIGH && !readyToClick) {
        readyToClick = true;
      }
    }
  } else if (readyToBroadcast) {
    uint8_t analogValue = map(analogRead(A0), 0, 1023, 0, 255);

    if (analogValue == 0 || analogValue == 255) {
      digitalWrite(5, HIGH);
    } else {
      digitalWrite(5, LOW);
    }
    
    if (broadcasting) {

    } else {

    }
  }
}
