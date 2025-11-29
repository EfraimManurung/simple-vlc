/*
    Visible Light Communication (VLC) Receiver
    ------------------------------------------------
    Reads bits using an LDR and reconstructs bytes sent
    by the LED-based transmitter.

    CHANGE IN THIS VERSION (v2.0.0):
      ✔ Collect characters until newline '\n'
      ✔ Compare full received message to expected string
      ✔ Compute BER per message
      ✔ Do not rely on fixed expected-length
      ✔ Much more robust against noise

    TRANSMISSION FORMAT (per character):
        Start bit : LOW (LED OFF)
        Data bits : 8 bits, LSB first
        Stop bit  : HIGH (LED ON)

    Author: Efraim Manurung
    email: efraim.manurung@gmail.com

    v2.0.0 (BER extension)
*/

#include <Arduino.h>

#define LDR_PIN A0
#define LIGHT_THRESHOLD 800
#define PERIOD 50 // must match transmitter timing

bool previous_light_state = false;
bool current_light_state = false;

/* -------------------------------------------------------------
   PROTOTYPES
------------------------------------------------------------- */
bool read_LDR_as_digital();
char read_byte_from_light();

/* -------------------------------------------------------------
   BER VARIABLES
------------------------------------------------------------- */
const char *EXPECTED_MESSAGE = "Hello World\n";
String rx_buffer = "";

unsigned long total_bits_received = 0;
unsigned long bit_errors = 0;
unsigned long messages_received = 0;
int count_how_many_payload_have_arrived = 0;

/* -------------------------------------------------------------
   SETUP
------------------------------------------------------------- */
void setup() {
  pinMode(LDR_PIN, INPUT);
  Serial.begin(9600);
  Serial.println("Receiver node with BER measurement started.");
}

/* -------------------------------------------------------------
   MAIN LOOP
------------------------------------------------------------- */
void loop() {

  current_light_state = read_LDR_as_digital();

  // Detect falling edge: HIGH → LOW => start bit detected
  if (previous_light_state == true && current_light_state == false) {

    char received_char = read_byte_from_light();

    if (received_char != 0) {
      rx_buffer += received_char;
      Serial.print(received_char); // real-time output
    }

    // ---------------------------------------------------------
    //     CHECK IF WE RECEIVED A FULL MESSAGE ("\n")
    // ---------------------------------------------------------
    if (received_char == '\n') {

      messages_received++;

      Serial.println("\n--- Full message received ---");
      Serial.print("Message: ");
      Serial.println(rx_buffer);

      // -------------------------------------------------------
      //                BIT ERROR CALCULATION
      // -------------------------------------------------------
      int expected_len = strlen(EXPECTED_MESSAGE);
      int received_len = rx_buffer.length();
      int min_len = min(expected_len, received_len);

      for (int i = 0; i < min_len; i++) {

        char expected_c = EXPECTED_MESSAGE[i];
        char received_c = rx_buffer[i];

        // Compare all 8 bits
        for (int b = 0; b < 8; b++) {

          bool exp_bit = (expected_c >> b) & 1;
          bool rec_bit = (received_c >> b) & 1;

          total_bits_received++;

          if (exp_bit != rec_bit)
            bit_errors++;
        }
      }

      float BER = (float)bit_errors / total_bits_received;

      Serial.printf("Bits compared: %lu\n", total_bits_received);
      Serial.printf("Bit errors  : %lu\n", bit_errors);
      Serial.printf("Current BER = %.6f\n", BER);
      Serial.println("-----------------------------------------");

      // reset buffer for next message
      rx_buffer = "";
    }
  }

  previous_light_state = current_light_state;
}

/* -------------------------------------------------------------
   Convert LDR analog → digital 0/1
------------------------------------------------------------- */
bool read_LDR_as_digital() {
  int val = analogRead(LDR_PIN);
  return (val > LIGHT_THRESHOLD);
}

/* -------------------------------------------------------------
   Read one full 8-bit char (start bit already detected)
------------------------------------------------------------- */
char read_byte_from_light() {

  char reconstructed = 0;

  // align to the middle of data bit 0
  delay(PERIOD * 1.5);

  for (int bit_index = 0; bit_index < 8; bit_index++) {

    bool bit_value = read_LDR_as_digital();

    reconstructed |= (bit_value << bit_index);

    delay(PERIOD);
  }

  return reconstructed;
}