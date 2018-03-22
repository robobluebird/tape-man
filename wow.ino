#include <Base64.h>
#include <global.h>
#include <sha1.h>
#include <WebSocketClient.h>
#include <SoftwareSerial.h>
#include <SparkFunESP8266WiFi.h>


const char mySSID[] = "WE WORK";
const char myPSK[] = "P@ssw0rd";

const char destServer[] = "echo.websocket.org";
const String httpRequest = "GET / HTTP/1.1\n"
                           "Host: echo.websocket.org\n"
                           "Connection: close\n\n";

WebSocketClient webSocketClient;

void setup()
{
  Serial.begin(9600);

  int test = esp8266.begin();

  if (test != true)
  {
    Serial.println(F("Error talking to ESP8266."));
    errorLoop(test);
  }
  Serial.println(F("ESP8266 Shield Present"));

  int retVal = esp8266.getMode();
  if (retVal != ESP8266_MODE_STA)
  { // If it's not in station mode.
    // Use esp8266.setMode([mode]) to set it to a specified
    // mode.
    retVal = esp8266.setMode(ESP8266_MODE_STA);
    if (retVal < 0)
    {
      Serial.println(F("Error setting mode."));
      errorLoop(retVal);
    }
  }
  Serial.println(F("Mode set to station"));

  // esp8266.status() indicates the ESP8266's WiFi connect
  // status.
  // A return value of 1 indicates the device is already
  // connected. 0 indicates disconnected. (Negative values
  // equate to communication errors.)
  retVal = esp8266.status();
  if (retVal <= 0)
  {
    Serial.print(F("Connecting to "));
    Serial.println(mySSID);
    // esp8266.connect([ssid], [psk]) connects the ESP8266
    // to a network.
    // On success the connect function returns a value >0
    // On fail, the function will either return:
    //  -1: TIMEOUT - The library has a set 30s timeout
    //  -3: FAIL - Couldn't connect to network.
    retVal = esp8266.connect(mySSID, myPSK);
    if (retVal < 0)
    {
      Serial.println(F("Error connecting"));
      errorLoop(retVal);
    }
  }

  char connectedSSID[24];
  memset(connectedSSID, 0, 24);
  // esp8266.getAP() can be used to check which AP the
  // ESP8266 is connected to. It returns an error code.
  // The connected AP is returned by reference as a parameter.
  retVal = esp8266.getAP(connectedSSID);
  if (retVal > 0)
  {
    Serial.print(F("Connected to: "));
    Serial.println(connectedSSID);
  }

  // esp8266.localIP returns an IPAddress variable with the
  // ESP8266's current local IP address.
  IPAddress myIP = esp8266.localIP();
  Serial.print(F("My IP: ")); Serial.println(myIP);
}

void loop()
{
  serialTrigger(F("Press any key to connect client."));
  clientDemo();
}

void clientDemo()
{
  // To use the ESP8266 as a TCP client, use the
  // ESP8266Client class. First, create an object:
  ESP8266Client client;
  
  int ret = client.connect(destServer, 80, 5000);

  Serial.println(String(ret));

  if (client.connected()) {
    Serial.println(F("Connected!"));
  } else {
    Serial.println(F("Failed to connect to server."));
    return;
  }

  // Handshake with the server
  webSocketClient.path = "/";
  webSocketClient.host = "echo.websocket.org";

  if (webSocketClient.handshake(client)) {
    Serial.println("Handshake successful");
  } else {
    Serial.println("Handshake failed.");
    while (1) {
      // Hang on failure
    }
  }

  if (client.connected()) {
    String data;

    webSocketClient.getData(data);

    if (data.length() > 0) {
      Serial.print("Received data: ");
      Serial.println(data);
    } else {
      Serial.println(F("Nothing to receive"));
    }

    // capture the value of analog 1, send it along
    pinMode(1, INPUT);
    
    data = String(analogRead(1));

    webSocketClient.sendData(data);
  } else {

    Serial.println("Client disconnected.");
    while (1) {
      // Hang on disconnect.
    }
  }

  // connected() is a boolean return value - 1 if the
  // connection is active, 0 if it's closed.
  if (client.connected())
    client.stop(); // stop() closes a TCP connection.
}

// errorLoop prints an error code, then loops forever.
void errorLoop(int error)
{
  Serial.print(F("Error: ")); Serial.println(error);
  Serial.println(F("Looping forever."));
  for (;;)
    ;
}

// serialTrigger prints a message, then waits for something
// to come in from the serial port.
void serialTrigger(String message)
{
  Serial.println();
  Serial.println(message);
  Serial.println();
  while (!Serial.available())
    ;
  while (Serial.available())
    Serial.read();
}
