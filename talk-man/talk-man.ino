#include <Ethernet.h>
#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <WebSocketsClient.h>

// Just make up a MAC address
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

WebSocketsClient webSocket;

IPAddress ip(10, 17, 4, 207);

char hex[64];
char* sprinter = hex;

void setup() {

  //  for (int i = 0; i < 8; i++) {
  //    pinMode(i, OUTPUT);
  //  }
  //

  pinMode(9, OUTPUT);

  Serial.begin(9600);

  while (!Serial) {
    ;
  }

  delay(1000); // give the ethernet module time to boot up:

  Serial.println(F("Connecting to ethernet..."));

  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    Ethernet.begin(mac, ip);
  }

  Serial.print("Connected. My IP address: ");
  Serial.println(Ethernet.localIP());

  makeConnection();
}

void makeConnection() {
  Serial.println(F("Connecting..."));
  webSocket.begin("tape-man.herokuapp.com", 80);
  webSocket.sendTXT("FF");
}

void loop() {
  Serial.println("wut");
  webSocket.sendTXT("FF");
  
  uint8_t analogValue = (analogRead(A0) + 1) / 4 - 1;

  if (analogValue < 0)
    analogValue = 0;

  if (analogValue == 0 || analogValue == 255) {
    digitalWrite(9, HIGH);
  } else {
    digitalWrite(9, LOW);
  }
  //
  //  sprinter += sprintf(sprinter, "%02X", analogValue);
  //
  //  if (sprinter >= hex + 64) {
  //    if (client.connected()) {
  //      webSocketClient.sendData(hex);
  //    } else {
  //      makeConnection();
  //    }
  //
  //    sprinter = hex;
  //  }
  //
  //  PORTD = analogValue;
}
