#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Encoder.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WebSocketClient.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

WiFiClient client;
WebSocketClient webSocketClient;

char server[] = "tape-man.herokuapp.com";
uint8_t bytes[250];
int byteIndex = 0;

char* pathPtr = "/";
char* hostPtr = server;

String networks[50];
int networkCount = 0;
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
  webSocketClient.path = pathPtr;
  webSocketClient.host = hostPtr;
  
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

  networkCount = WiFi.scanNetworks();

  networkCount = networkCount > 50 ? 50 : networkCount;

  if (networkCount == 0) {
    resetDisplay();

    display.println("No networks found.");
  } else {
    for (int i = 0; i < networkCount; i++) {
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

    if (i < networkCount) {
      display.println(networks[i]);
    } else {
      display.println("");
    }
  }

  display.display();
}

void shiftUp(int amount = 1) {
  if (selectedNetworkIndex + amount >= networkCount) {
    selectedNetworkIndex = networkCount - 1;
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

  display.println("enter password for:");
  display.println(ssid);

  if (password.length() > 21) {
    display.println(password.substring(password.length() - 21));
  } else {
    display.println(password);
  }

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

  showPasswordSelection();
}

void showError(String error) {
  resetDisplay();
  display.println(error);
  display.display();
}

void showBroadcasting(bool b = false) {
  selectingPassword = false;
  selectingNetwork = false;
  readyToBroadcast = true;
  broadcasting = b;

  resetDisplay();

  display.println("/////////////////////");
  display.println(broadcasting ? "broadcasting!" : "not broadcasting yet");
  display.display();
}

void connectToNetwork() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  resetDisplay();

  display.println("connecting...");
  display.display();

  WiFi.begin(ssid.c_str(), password.c_str());

  int attempts = 0;

  display.setTextWrap(true);
  
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    display.print(".");
    display.display();
    attempts++;
  }

  display.setTextWrap(false);

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
      showBroadcasting();
    } else {
      showError(":(");
    }
  }
}

void showBroadcastingConnectionAttempt(String message = "confused...") {
  resetDisplay();

  display.println("/////////////////////");
  display.println(message);

  display.display();
}

void startBroadcasting() {
  byteIndex = 0;
  
  showBroadcastingConnectionAttempt("finding tape-man...");

  int tries = 0;

  while (!client.connect(server, 80) && tries <= 10) {
    showBroadcastingConnectionAttempt("failed, retry in 3...");
    delay(1000);
    display.println("2...");
    display.display();
    delay(1000);
    display.println("1...");
    display.display();
    tries++;
  }

  if (tries == 10) {
    showError("totally failed.");
    while (1) {}
  }

  showBroadcastingConnectionAttempt("connected.");
  display.println("shaking hands...");
  display.display();

  if (webSocketClient.handshake(client)) {
    showBroadcasting(true);
  } else {
    showBroadcastingConnectionAttempt("handshake failed");
    delay(2000);
    showBroadcasting(false);
  }
}

void stopBroadcasting() {
  showBroadcasting(false);
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
      bytes[byteIndex] = analogValue;

      if (byteIndex >= 249) {
        if (client.connected()) {
          webSocketClient.sendData(bytes, sizeof(bytes));
        } else {
          Serial.println(F("I'm sorry that you have failed...trying again in 3..."));
          delay(1000);
          Serial.println(F("2..."));
          delay(1000);
          Serial.println(F("1..."));
          startBroadcasting();
        }

        byteIndex = 0;
      } else {
        byteIndex++;
      }
      
      if (digitalRead(12) == HIGH && readyToPress) {
        readyToPress = false;
        stopBroadcasting();
      } else if (digitalRead(12) == LOW && !readyToPress) {
        readyToPress = true;
      }
    } else {
      if (digitalRead(12) == HIGH && readyToPress) {
        readyToPress = false;
        startBroadcasting();
      } else if (digitalRead(12) == LOW && !readyToPress) {
        readyToPress = true;
      }
    }
  }
}
