#include <Base64.h>
#include <global.h>
#include <sha1.h>
#include <WebSocketClient.h>
#include <Ethernet.h>

EthernetClient client;
WebSocketClient webSocketClient;

// Just make up a MAC address
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
char server[] = "10.17.108.5";
uint8_t bytes[125];
int byteIndex = 0;

char wsPath[] = "/";
char* pathPtr = wsPath;
char* hostPtr = server;

void setup() {
  webSocketClient.path = pathPtr;
  webSocketClient.host = hostPtr;

  //  setup output pins for DAC
  for (int i = 0; i < 8; i++) {
    pinMode(i, OUTPUT);
  }

  // clipping indicator
  pinMode(9, OUTPUT);

  Serial.begin(9600);

  while (!Serial) {}

  delay(1000); // give the ethernet module time to boot up

  Serial.println(F("Connecting to ethernet..."));

  Ethernet.begin(mac);

  Serial.print(F("Connected. My IP address: "));
  Serial.println(Ethernet.localIP());

  makeConnection();
}

void makeConnection() {
  Serial.println(F("Connecting to tape-man..."));

  if (client.connect(server, 80)) {
    Serial.println(F("Connected."));
    Serial.println(F("Shaking hands..."));
    
    if (webSocketClient.handshake(client)) {
      Serial.println("Handshake successful");
    } else {
      Serial.println("Handshake failed.");
      while (1) {}
    }
  } else {
    Serial.println("Connection failed.");
  }
}

void loop() {
  uint8_t analogValue = (analogRead(A0) + 1) / 4 - 1;

  if (analogValue < 0)
    analogValue = 0;

  if (analogValue == 0 || analogValue == 255) {
    digitalWrite(9, HIGH);
  } else {
    digitalWrite(9, LOW);
  }

  bytes[byteIndex] = analogValue;

  if (byteIndex >= 124) {
    if (client.connected()) {
      webSocketClient.sendData(bytes, sizeof(bytes));
    } else {
      Serial.println(F("I'm sorry that you have failed"));
      while (1) {}
    }

    byteIndex = 0;
  } else {
    byteIndex++;
  }

  //  // send audio out to ports 1-7 of arduino, which feed into 8-bit DAC
  //  // PORTD is all of those 8 digital outs put together, for some reason
  //  PORTD = analogValue;
}
