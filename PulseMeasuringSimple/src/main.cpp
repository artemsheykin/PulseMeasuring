#include <Arduino.h>

const int pulsePin = 21; // pin number where the pulse will be generated
const int measurementPin = 22; // pin number to measure the pulse duration

volatile bool pulseState = HIGH;
volatile bool pulseMeasurementStarted = false;
volatile uint64_t pulseStartTime = 0;
volatile float pulseDuration = 0;

hw_timer_t *pulseTimer = NULL;
hw_timer_t *measureTimer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR pulseInterrupt() {
  portENTER_CRITICAL_ISR(&timerMux);
  pulseState = !pulseState; // invert pin state, basically square wave with 50% duty cycle
  digitalWrite(pulsePin, pulseState);
  portEXIT_CRITICAL_ISR(&timerMux);
}

void setup() {
  Serial.begin(115200);

  pinMode(pulsePin, OUTPUT);
  pinMode(measurementPin, INPUT_PULLUP);

  // Configure the pulseTimer
  pulseTimer = timerBegin(0, 80, true); // timer 0, timer speed = 1 MHz, counting up
  timerAttachInterrupt(pulseTimer, &pulseInterrupt, true); // true for edge-triggered
  timerAlarmWrite(pulseTimer, 10000, true); // repeating
  timerAlarmEnable(pulseTimer);

  // Confugure the measureTimer
  measureTimer = timerBegin(1, 2, true);  // timer 1, timer speed = 40 MHz (can't go lower), counting up
}

void loop() {
  // Measure the pulse duration
  if (digitalRead(measurementPin) == HIGH && !pulseMeasurementStarted) {
    pulseStartTime = timerRead(measureTimer);
    pulseMeasurementStarted = true;
  } else if (digitalRead(measurementPin) == LOW && pulseMeasurementStarted) {
    pulseDuration = static_cast<float>(timerRead(measureTimer) - pulseStartTime) / 80.0f; //resolution is 25 ns
    pulseMeasurementStarted = false;
    Serial.print("Pulse duration: ");
    Serial.print(pulseDuration);
    Serial.println(" microseconds");
  }
}




