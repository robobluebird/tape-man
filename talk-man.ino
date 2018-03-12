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

IPAddress ip(192, 168, 1, 153);
EthernetClient client;
WebSocketClient webSocketClient;

char server[] = "192.168.1.151";
char hex[64];
char* sprinter = hex;

void setup() {
  for (int i = 0; i < 8; i++) {
    pinMode(i, OUTPUT);
  }

  Serial.begin(9600);

  while (!Serial) {
    ;
  }

  delay(1000); // give the ethernet module time to boot up:

  // start the Ethernet connection using a fixed IP address and DNS server:
  Ethernet.begin(mac, ip);

  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());

  makeConnection();
}

void makeConnection() {
  Serial.println("hello");

  // Connect to the websocket server
  if (client.connect(server, 80)) {
    Serial.println("Connected");
  } else {
    Serial.println("Connection failed.");
    while (1) {
      // Hang on failure
    }
  }

  // Handshake with the server
  char path[] = "/";
  char* pathPtr = path;
  char host[] = "ws://192.168.1.151";
  char* hostPtr = host;
  webSocketClient.path = pathPtr;
  webSocketClient.host = hostPtr;

  if (webSocketClient.handshake(client)) {
    Serial.println("Handshake successful");
  } else {
    Serial.println("Handshake failed.");
    while (1) {
      // Hang on failure
    }
  }
}

void loop() {
  uint8_t analogValue = (analogRead(A0) + 1) / 4 - 1;

  if (analogValue < 0)
    analogValue = 0;

  if (analogValue == 0 || analogValue == 255) {
    digitalWrite(13, HIGH);
  } else {
    digitalWrite(13, LOW);
  }

  sprinter += sprintf(sprinter, "%02X", analogValue);

  if (sprinter >= hex + 64) {
    if (client.connected()) {
      webSocketClient.sendData(hex);
    } else {
      makeConnection();
    }

    sprinter = hex;
  }

  PORTD = analogValue;
}
