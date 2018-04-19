//#include <Base64.h>
//#include <global.h>
//#include <sha1.h>

#include <ESP8266WiFi.h>
#include <WebSocketClient.h>

WiFiClient client;
WebSocketClient webSocketClient;

char server[] = "tape-man.herokuapp.com";
uint8_t bytes[1000];
int byteIndex = 0;

char wsPath[] = "/";
char* pathPtr = wsPath;
char* hostPtr = server;

// const char* ssid = "WE WORK";
// const char* password = "P@ssw0rd";

const char* ssid = "Fios-HEUK7";
const char* password = "ton4660bug682deck";

void setup() {
  webSocketClient.path = pathPtr;
  webSocketClient.host = hostPtr;

  // clipping indicator
  pinMode(5, OUTPUT);

  Serial.begin(115200);
  while (!Serial) {}

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  makeConnection();
}

void makeConnection() {
  Serial.println(F("Connecting to tape-man..."));

  int tries = 0;

  while (!client.connect(server, 80) && tries <= 10) {
    Serial.println(F("Failed. Trying again in 3..."));
    delay(1000);
    Serial.println(F("2..."));
    delay(1000);
    Serial.println(F("1..."));
    tries++;
  }

  if (tries == 10) {
    Serial.println(F("Totally failed"));
    while (1) {}
  }

  Serial.println(F("Connected."));
  Serial.println(F("Shaking hands..."));

  if (webSocketClient.handshake(client)) {
    Serial.println("Handshake successful");
  } else {
    Serial.println("Handshake failed.");
    while (1) {}
  }
}

void loop() {
  uint8_t analogValue = map(analogRead(A0), 0, 1023, 0, 255);

  if (analogValue == 0 || analogValue == 255) {
    digitalWrite(5, HIGH);
  } else {
    digitalWrite(5, LOW);
  }

  bytes[byteIndex] = analogValue;

  if (byteIndex >= 999) {
    if (client.connected()) {
      webSocketClient.sendData(bytes, sizeof(bytes));
    } else {
      Serial.println(F("I'm sorry that you have failed...trying again in 3..."));
      delay(1000);
      Serial.println(F("2..."));
      delay(1000);
      Serial.println(F("1..."));
      makeConnection();
    }

    byteIndex = 0;
  } else {
    byteIndex++;
  }
}
