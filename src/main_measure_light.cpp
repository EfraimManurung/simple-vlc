// -----------------------------------------------------------------------------
// File: main_measure_light.cpp
// Description: ESP8266 non-blocking ADC sampling with moving-average + percent
// -----------------------------------------------------------------------------

#include <Arduino.h>

#define LDR_PIN A0

static const uint32_t SAMPLE_INTERVAL_MS = 50;
static const int NUM_SAMPLES = 10;

volatile int raw_adc_value = 0;
long sum_samples = 0;
int sample_count = 0;
int sample_buffer[NUM_SAMPLES];

uint32_t last_sample_time = 0;

// -----------------------------------------------------------------------------
// Fast periodic sampling via timer1 ISR
// -----------------------------------------------------------------------------
void ICACHE_RAM_ATTR onTimerISR() { raw_adc_value = analogRead(LDR_PIN); }

// -----------------------------------------------------------------------------
// SETUP
// -----------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  delay(200);

  for (int i = 0; i < NUM_SAMPLES; i++)
    sample_buffer[i] = 0;

  timer1_attachInterrupt(onTimerISR);
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_LOOP);
  timer1_write(20000);

  Serial.println("ADC sampling initialized.");
}

// -----------------------------------------------------------------------------
// MAIN LOOP
// -----------------------------------------------------------------------------
void loop() {
  uint32_t now = millis();

  if (now - last_sample_time >= SAMPLE_INTERVAL_MS) {

    int index = sample_count % NUM_SAMPLES;

    sum_samples -= sample_buffer[index];
    sample_buffer[index] = raw_adc_value;
    sum_samples += raw_adc_value;

    sample_count++;

    int used = (sample_count < NUM_SAMPLES) ? sample_count : NUM_SAMPLES;
    float average = (float)sum_samples / used;

    // ---- Raw % (0 = bright, 100 = dark) ----
    float raw_percent = (raw_adc_value / 1023.0f) * 100.0f;
    float avg_percent = (average / 1023.0f) * 100.0f;

    // ---- Inverted % (0 = dark, 100 = bright) ----
    float raw_brightness = 100.0f - raw_percent;
    float avg_brightness = 100.0f - avg_percent;

    Serial.printf("RAW=%d (%5.1f%% dark | %5.1f%% bright)   "
                  "AVG=%.2f (%5.1f%% dark | %5.1f%% bright)\n",
                  raw_adc_value,
                  raw_percent,    // dark %
                  raw_brightness, // bright %
                  average,
                  avg_percent,   // dark %
                  avg_brightness // bright %
    );

    last_sample_time = now;
  }
}