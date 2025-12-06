/*
    Visible Light Communication (VLC) Transmitter
    ------------------------------------------------
    Sends characters using LED ON/OFF pulses as serial bits.
    Format per transmitted character:
        Start bit:  LOW
        8 data bits: LSB first
        Stop bit:   HIGH

    Data is sent using Manchester-like LED toggling,
    but simplified as raw ON/OFF bit signals.

    String transmitted in this example:
        "Hello World!"

    This code explains:
        - How each ASCII character becomes binary
        - How bits are transmitted using LED
        - How the receiver decodes the arriving bits
        - Why LSB-first matters
        - Full bit breakdown for characters like 'H'

    Author: Efraim Manurung
    email: efraim.manurung@gmail.com
*/

#include <Arduino.h>

#define LED_PIN 2
#define PERIOD                                                                 \
  50 // duration of each bit in ms (transmitter + receiver must match timing)

#define SEND_X_TIMES 10

char *message_string =
    "Hello World\n"; // message to transmit, we have to add newline!
// Now message_length = 12 bytes of data, and the last byte = ASCII 10.
int message_length;

/* function prototype */
void send_byte(char byte_to_send);

void setup() {
  pinMode(LED_PIN, OUTPUT);

  message_length = strlen(message_string);

  Serial.begin(9600);
  Serial.println("Transmitter node started.");

  /* make it stable */
  delay(10000);

  /* send data for X times */
  for (int send_idx = 0; send_idx < SEND_X_TIMES; send_idx++) {
    Serial.printf("SEND_X_TIME: %d\n", send_idx);
    for (int char_idx = 0; char_idx < message_length; char_idx++) {
      char outgoing_char = message_string[char_idx];

      // Print the character being transmitted for debugging
      Serial.print("Transmitting char: ");
      Serial.print(outgoing_char);
      Serial.print("   ASCII: ");
      Serial.println((int)outgoing_char);

      send_byte(outgoing_char);
    }

    /* waiting repeater to send the message */
    delay(10000);
  }

  Serial.println("STOPPED!");
}

/*
 ------------------------------------------------------------------------------
                                MAIN LOOP
------------------------------------------------------------------------------
*/
void loop() { /* not used */ delay(1000); }

/*
 ------------------------------------------------------------------------------
                       SEND ONE BYTE (8 BITS + START + STOP)
------------------------------------------------------------------------------
    Every byte is transmitted in this exact order:

        Start bit    = 0  (LED OFF)
        Bit 0 (LSB)  = extracted from char
        Bit 1
        Bit 2
        Bit 3
        Bit 4
        Bit 5
        Bit 6
        Bit 7 (MSB)
        Stop bit     = 1  (LED ON)

    Example for 'H' (ASCII 72):

            H  → decimal 72
               → binary (MSB→LSB): 01001000
               → transmitted (LSB→MSB): 00010010

    Receiver reconstruction matches this by:
        sample bit 0 → goes into position 0
        sample bit 1 → goes into position 1
        ...
        sample bit 7 → goes into position 7

 ------------------------------------------------------------------------------
*/
void send_byte(char byte_to_send) {
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

    // For debugging clarity in Serial Monitor:
    Serial.print("Bit ");
    Serial.print(bit_index);
    Serial.print(" (LSB=0): ");
    Serial.println(bit_value);
  }

  /*
      --------------------------------------------------
      STOP BIT (always HIGH)
      --------------------------------------------------
      This tells the receiver the character is complete.
  */
  digitalWrite(LED_PIN, HIGH);
  delay(PERIOD);

  Serial.println("---- End of byte ----");
}