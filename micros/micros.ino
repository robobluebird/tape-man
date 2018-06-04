#define _TASK_MICRO_RES

#include <TaskScheduler.h>

Scheduler runner;

void sampleCallback();
void transmitCallback();

unsigned long sampleInterval = 125L;

uint8_t sampleBuffer[8000];
uint8_t transmitBuffer[8000];
int byteIndex = 0;

Task sample(sampleInterval, TASK_FOREVER, &sampleCallback, &runner, true);
Task transmit(TASK_SECOND, TASK_FOREVER, &transmitCallback, &runner, true);

void sampleCallback() {
  uint8_t analogValue = map(analogRead(A0), 0, 1023, 0, 255);

  if (analogValue == 0 || analogValue == 255) {
    digitalWrite(5, HIGH);
  } else {
    digitalWrite(5, LOW);
  }

  sampleBuffer[byteIndex] = analogValue;

  if (byteIndex >= 7999) {
    memcpy(transmitBuffer, sampleBuffer, 8000);
    Serial.println("copied!");
    byteIndex = 0;
  } else {
    byteIndex++;
  }
}

void transmitCallback() {
  Serial.println("Would transmit here!");
}


void setup () {
  Serial.begin(115200);
  Serial.println("Scheduler Sampler");

  pinMode(5, OUTPUT);

  Serial.println("3...");
  delay(500);
  Serial.println("2...");
  delay(500);
  Serial.println("1...");
  delay(500);
  Serial.println("GO");

  runner.enableAll();
}


void loop () {
  runner.execute();
}
