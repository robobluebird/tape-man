#define _TASK_MICRO_RES
#define _TASK_STATUS_REQUEST
#define WS_FIN 0x80
#define WS_MASK 0x80
#define WS_SIZE16 126
#define WS_OPCODE_BINARY 0x02

#include <Button.h>
#include <TaskScheduler.h>
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

Scheduler runner;
StatusRequest doTransmit;
Encoder myEnc(0, 4);

void rotaryCallback();
void buttonCallback();
void sampleCallback();
void transmitCallback();

void shiftUp(int scroll);
void shiftDown(int scroll);
void shiftLeft(int scroll);
void shiftRight(int scroll);

unsigned long sampleInterval = 333UL;

uint8_t sampleBuffer[3000];
uint8_t transmitBuffer[3000];
int byteIndex = 0;
int counter = 1;
int scroll = 1;

// char server[] = "10.17.108.214";
// char server[] = "172.20.10.5";
// char server[] = "10.17.110.148";
char server[] = "tape-man.herokuapp.com";
char* pathPtr = "/";
char* hostPtr = server;

String networks[50];
int networkCount = 0;
int selectedNetworkIndex = 0;
int selectedCharacterIndex = 0;
String ssid = "";
String password = "";
int oldPosition  = 0;
int lastScrollTime = millis();
bool selectingNetwork = false;
bool selectingPassword = false;
bool readyToBroadcast = false;
bool broadcasting = false;
const String chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-_+=~`[]{}|\\:;\"'<>,.?/";

ButtonCB back(16);
ButtonCB center(13);
ButtonCB forward(12);

Task rotary(TASK_MILLISECOND, TASK_FOREVER, &rotaryCallback, &runner, true);
Task buttons(TASK_MILLISECOND, TASK_FOREVER, &buttonCallback, &runner, true);
Task sample(sampleInterval, TASK_FOREVER, &sampleCallback, &runner, false);
Task transmit(TASK_IMMEDIATE, TASK_ONCE, &transmitCallback, &runner, false);

void onBackClick(const Button& b) {
  if (selectingNetwork) {
  } else if (selectingPassword) {
    deleteCharacter();
  } else if (readyToBroadcast) {
  }
}

void onCenterClick(const Button& b) {
  if (selectingNetwork) {\
    selectNetwork();
  } else if (selectingPassword) {
    connectToNetwork();
  } else if (readyToBroadcast) {
    if (broadcasting) {
      stopBroadcasting();
    } else {
      startBroadcasting();
    }
  }
}

void onForwardClick(const Button& b) {
  if (selectingNetwork) {
  } else if (selectingPassword) {
    selectCharacter();
  } else if (readyToBroadcast) {
  }
}

void buttonCallback() {
  back.process();
  center.process();
  forward.process();
}

void rotaryCallback() {
  int newPosition = myEnc.read();

  if (newPosition != oldPosition && newPosition % 4 == 0) {
    if (newPosition > oldPosition) {
      if (selectingNetwork) {
        shiftUp(scroll);
      } else if (selectingPassword) {
        shiftLeft(scroll);
      }
    } else {
      if (selectingNetwork) {
        shiftDown(scroll);
      } else if (selectingPassword) {
        shiftRight(scroll);
      }
    }

    oldPosition = newPosition;
  }
}

void sampleCallback() {
  uint8_t analogValue = map(analogRead(A0), 0, 1023, 0, 255);

  if (analogValue == 0 || analogValue == 255) {
    digitalWrite(5, HIGH);
  } else {
    digitalWrite(5, LOW);
  }

  sampleBuffer[byteIndex] = analogValue;

  if (byteIndex >= 2999) {
    memcpy(transmitBuffer, sampleBuffer, 3000);
    byteIndex = 0;
    doTransmit.signal();
  } else {
    byteIndex++;
  }
}

long last = millis();

void transmitCallback() {
  Serial.println("bip");
  Serial.println(millis() - last);
  last = millis();
  sendData(transmitBuffer, sizeof(transmitBuffer));
  prepareForSignal();
}

void prepareForSignal() {
  doTransmit.setWaiting(1);
  transmit.waitFor(&doTransmit);
}

void showStatus() {
  showBroadcasting(true);
  counter++;
}

void setup () {
  webSocketClient.path = pathPtr;
  webSocketClient.host = hostPtr;

  pinMode(5, OUTPUT);

  Serial.begin(115200);

  EEPROM.begin(512);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  delay(100);
  display.clearDisplay();
  display.setTextWrap(false);
  resetDisplay();
  display.println("hello.");
  display.display();
  delay(2000);

  back.setClickHandler(onBackClick);
  center.setClickHandler(onCenterClick);
  forward.setClickHandler(onForwardClick);

  doTransmit.setWaiting(1);
  transmit.waitFor(&doTransmit);

  runner.disableAll();

  buttons.enable();
  rotary.enable();

  if (readConnection()) {
    connectToNetwork();
  } else {
    scanNetworks();
  }
}

void loop () {
  runner.execute();
}

void showBroadcasting(bool b) {
  selectingPassword = false;
  selectingNetwork = false;
  readyToBroadcast = true;
  broadcasting = b;

  resetDisplay();

  display.println("/////////////////////");
  display.println(broadcasting ? "" : "not broadcasting yet");

  if (broadcasting) {
    display.setTextSize(2);
    display.println(counter);
    display.setTextSize(1);
  }

  display.display();
}

void scanNetworks() {
  resetDisplay();

  display.println();
  display.println("scanning networks...");
  display.display();

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

  index++;
  currentChar = char(EEPROM.read(index));

  while (currentChar != '\0') {
    password += currentChar;
    index++;
    currentChar = char(EEPROM.read(index));
  }

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
  display.print(ssid);
  display.println(":");

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
  int16_t x = display.getCursorX();
  int16_t y = display.getCursorY();
  display.setCursor(x, y - 8);
  display.setTextSize(2);
  display.print(chars.charAt(selectedCharacterIndex));
  x = display.getCursorX();
  display.setCursor(x, y);
  display.setTextSize(1);
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

void deleteCharacter() {
  if (password.length() > 0) {
    password.remove(password.length() - 1);
    showPasswordSelection();
  } else {
    scanNetworks();
  }
}

void selectNetwork() {
  ssid = networks[selectedNetworkIndex];

  selectedCharacterIndex = 0;
  showPasswordSelection();
}

void showError(String error) {
  resetDisplay();
  display.println(error);
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

  while ((WiFi.status() != WL_CONNECTED && attempts < 30) && WiFi.status() != WL_CONNECT_FAILED) {
    delay(500);
    display.print(".");
    display.display();
    attempts++;
  }

  display.setTextWrap(false);

  if (WiFi.status() == WL_CONNECT_FAILED) {
    resetDisplay();
    display.println();
    display.println("Incorrect password");
    display.display();
    delay(1000);
    showPasswordSelection();
  } else if (WiFi.status() != WL_CONNECTED) {
    resetDisplay();
    display.println();
    display.println("Failed to connect");
    display.display();
    delay(1000);
    ssid = "";
    password = "";
    scanNetworks();
  } else {
    if (writeConnection()) {
      stopBroadcasting();
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
    runner.startNow();
    runner.enableAll();

    showBroadcasting(true);
  } else {
    stopBroadcasting();

    showBroadcastingConnectionAttempt("handshake failed");
    delay(2000);

    showBroadcasting(false);
  }
}

void stopBroadcasting() {
  runner.disableAll();
  runner.startNow();
  buttons.enable();
  rotary.enable();
  sample.enable();

  counter = 1;

  showBroadcasting(false);
}

void handleNetworkSelection() {
  int newPosition = myEnc.read();

  if (newPosition != oldPosition && newPosition % 4 == 0) {
    if (newPosition > oldPosition) {
      shiftUp(scroll);
    } else {
      shiftDown(scroll);
    }

    oldPosition = newPosition;
  }
}

void handlePasswordSelection() {
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
  }
}

void sendData(uint8_t *bytes, size_t size) {
  uint8_t mask[4];
  int buffer_size = size + 1;
  int buffer_index = 0;

  if (size > 125) {
    buffer_size += 3;
  } else {
    buffer_size += 1;
  }

  if (WS_MASK > 0) {
    buffer_size += 4;
  }

  uint8_t buffer[buffer_size];

  buffer[buffer_index++] = (uint8_t) (WS_OPCODE_BINARY | WS_FIN);
  
  if (size > 125) {
    buffer[buffer_index++] = (uint8_t) (WS_SIZE16 | WS_MASK);
    buffer[buffer_index++] = (uint8_t) (size >> 8);
    buffer[buffer_index++] = (uint8_t) (size & 0xFF);
  } else {
    buffer[buffer_index++] = (uint8_t) (size | WS_MASK);
  }

  if (WS_MASK > 0) {
    mask[0] = random(0, 256);
    mask[1] = random(0, 256);
    mask[2] = random(0, 256);
    mask[3] = random(0, 256);

    buffer[buffer_index++] = mask[0];
    buffer[buffer_index++] = mask[1];
    buffer[buffer_index++] = mask[2];
    buffer[buffer_index++] = mask[3];

    for (int i=0; i<size; ++i) {
      bytes[i] = bytes[i] ^ mask[i % 4];
    }
  }

  for (int i = 0; i < size; i++) {
    buffer[buffer_index++] = bytes[i];
  }
  
  client.write(buffer, buffer_size);
}
