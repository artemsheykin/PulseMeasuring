#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

const int pulsePin = 21; // pin for sending the pulse
const int measurePin = 22; // pin for measuring the pulse

unsigned long startTime;
unsigned long endTime;
bool measurementstarted = false;

// Task to send the pulse on a specific pin 
void sendPulseTask(void *parameter) {
  for (;;) {  
    pinMode(pulsePin, OUTPUT);  // set up pin for output
    digitalWrite(pulsePin, HIGH); // start pulse
    delayMicroseconds(1000); // adjust pulse length as needed
    digitalWrite(pulsePin, LOW);  // end pulse
    vTaskDelay(1); // add a delay to avoid excessive CPU usage (crashes without)
  }
}


void setup() {

  Serial.begin(115200);

  pinMode(measurePin, INPUT_PULLDOWN); // set up pin for input, default state is low

  // create and run pulse sending task on different core
  xTaskCreatePinnedToCore(sendPulseTask, "SendPulseTask", 1000, NULL, 1, NULL, 0); // task runs on core 0

}

void loop() {
  // loop() runs on core 1, using loop to run pulse measuring task

  while (digitalRead(measurePin) == LOW && measurementstarted == false) {} // waiting for pulse

  
  // pulse detected
  if (digitalRead(measurePin) == HIGH && measurementstarted == false) {
    startTime = micros(); // timestamp
    measurementstarted = true;
  }

  while (digitalRead(measurePin) == HIGH && measurementstarted == true){} // wait for pulse to end

  
  // pulse ended (pin went low again)
  if (digitalRead(measurePin) == LOW && measurementstarted == true) {
    endTime = micros(); // timestamp
    measurementstarted = false; // reset bool
    
    unsigned long pulseLength = endTime - startTime;
    Serial.println(pulseLength);
  }
}