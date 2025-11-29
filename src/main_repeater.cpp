// file: repeater_dynamic_fixed.ino

#include <Arduino.h>

#define LDR_PIN A0
#define LED_PIN 2
#define LIGHT_THRESHOLD 800
#define PERIOD 50

String received_buffer = "";

bool previous_light_state = false;
unsigned long last_bit_time = 0;
bool is_sending = false;

/* ------------------------------------------------------------- */

bool read_LDR_as_digital() { return analogRead(LDR_PIN) > LIGHT_THRESHOLD; }

/* ------------------------------------------------------------- */

char read_byte_from_light() {

  char reconstructed = 0;

  // align sampling to middle of first data bit
  delay(PERIOD * 1.5);

  for (int bit_index = 0; bit_index < 8; bit_index++) {
    bool bit_value = read_LDR_as_digital();
    reconstructed |= (bit_value << bit_index);
    delay(PERIOD);
  }

  return reconstructed;
}

/* ------------------------------------------------------------- */

void send_byte(char byte_to_send) {
  is_sending = true;
  /*
      --------------------------------------------------
      START BIT (always LOW)
      --------------------------------------------------
      The receiver detects LOW after a HIGH and knows:
      "A new character is starting now".
  */
  digitalWrite(LED_PIN, LOW);
  delay(PERIOD);

  /*
      --------------------------------------------------
      SEND ALL 8 DATA BITS (LSB FIRST)
      --------------------------------------------------

      We extract one bit at a time using:

          mask = (1 << bit_index);
          bit_value = (byte_to_send & mask) != 0;

      Meaning:
          If byte_to_send = 01001000 ('H')
          mask for bit_index 3 = 00001000

          (01001000 & 00001000) = 00001000 → bit = 1

      This is exactly how serial UART works internally.
  */
  for (int bit_index = 0; bit_index < 8; bit_index++) {
    int mask = (1 << bit_index);                 // isolate one bit position
    bool bit_value = (byte_to_send & mask) != 0; // extract bit (true = 1)

    /*
        When bit_value is:
            0 → LED OFF
            1 → LED ON
    */
    digitalWrite(LED_PIN, bit_value);
    delay(PERIOD);
  }

  /*
      --------------------------------------------------
      STOP BIT (always HIGH)
      --------------------------------------------------
      This tells the receiver the character is complete.
  */
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

  // complete message when newline received
  if (received_buffer.endsWith("\n")) {

    for (int i = 0; i < received_buffer.length(); i++) {
      char c = received_buffer[i];
      Serial.print("Transmitting: ");
      Serial.println(c);
      send_byte(c);
    }

    received_buffer = "";
  }
}