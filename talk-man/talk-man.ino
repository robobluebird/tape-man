#include <Base64.h>
#include <global.h>
#include <sha1.h>
#include <SPI.h>
#include <Ethernet.h>
#include <WebSocketClient.h>

// Just make up a MAC address
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

EthernetClient client;
WebSocketClient webSocketClient;
IPAddress ip(10, 17, 4, 207);

char server[] = "tape-man.herokuapp.com";
char hex[100];
char* sprinter = hex;
char wsPath[] = "/";
char* pathPtr = wsPath;
char* hostPtr = server;

void setup() {
  webSocketClient.path = pathPtr;
  webSocketClient.host = hostPtr;

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

  Ethernet.begin(mac);

  Serial.print("Connected. My IP address: ");
  Serial.println(Ethernet.localIP());

  makeConnection();
}

void makeConnection() {
  Serial.println(F("Connecting..."));

  if (client.connect(server, 80)) {
    Serial.println(F("Connected"));

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

  sprinter += sprintf(sprinter, "%02X", analogValue);

  if (sprinter >= hex + 100) {
    if (client.connected()) {
      webSocketClient.sendData(hex);
    } else {
      Serial.println(F("I'm sorry that you have failed"));
      while(1) {}
    }

    sprinter = hex;
  }

//  PORTD = analogValue;
}
