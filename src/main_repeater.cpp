// file: repeater_dynamic_fixed.ino

#include <Arduino.h>

#define LDR_PIN A0
#define LED_PIN 2
#define LIGHT_THRESHOLD 800
#define PERIOD 50
#define CHAR_TIMEOUT 300

String received_buffer = "";

bool previous_light_state = false;
unsigned long last_bit_time = 0;
bool is_sending = false;

/* ------------------------------------------------------------- */

bool read_LDR_as_digital() { return analogRead(LDR_PIN) > LIGHT_THRESHOLD; }

/* ------------------------------------------------------------- */

char read_byte_from_light() {
  // verify real start bit by resampling
  delay(PERIOD / 2);

  if (read_LDR_as_digital() == true) {
    return 0; // false trigger, abort
  }

  char reconstructed = 0;

  // align to middle of bit 0
  delay(PERIOD);

  for (int bit_index = 0; bit_index < 8; bit_index++) {
    bool bit_value = read_LDR_as_digital();
    reconstructed |= (bit_value << bit_index);
    delay(PERIOD);
  }

  return reconstructed;
}

/* ------------------------------------------------------------- */

void send_byte(char b) {
  is_sending = true;

  digitalWrite(LED_PIN, LOW);
  delay(PERIOD);

  for (int i = 0; i < 8; i++) {
    digitalWrite(LED_PIN, (b >> i) & 1);
    delay(PERIOD);
  }

  digitalWrite(LED_PIN, HIGH);
  delay(PERIOD);

  is_sending = false;
}

/* ------------------------------------------------------------- */

void setup() {
  pinMode(LDR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(9600);
}

/* ------------------------------------------------------------- */

void loop() {

  if (!is_sending) {
    bool current_light_state = read_LDR_as_digital();

    // require stable HIGH before falling edge
    if (previous_light_state == true && current_light_state == false) {

      char c = read_byte_from_light();

      if (c != 0) { // ignore false triggers
        received_buffer += c;
        Serial.print("Received: ");
        Serial.println(c);
        last_bit_time = millis();
      }
    }

    previous_light_state = current_light_state;
  }

  // complete message when quiet for a while
  if (received_buffer.length() > 0 && millis() - last_bit_time > CHAR_TIMEOUT) {

    Serial.println("--- Message complete ---");

    for (int i = 0; i < received_buffer.length(); i++) {
      char c = received_buffer[i];
      Serial.print("Transmitting: ");
      Serial.println(c);
      send_byte(c);
    }

    received_buffer = ""; // clear for next message
    delay(200);           // allow LDR to settle
  }
}