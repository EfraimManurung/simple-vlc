#include <Arduino.h>

#define LDR_PIN A0
#define LIGHT_THRESHOLD 800

/* how to make it dynamic */
#define MAX_BYTES 11 // "Hello World"

#define LED_PIN 2
#define PERIOD 50

char received_buffer[MAX_BYTES];
int buffer_index = 0;
int message_length;

/* function prototype */
void send_byte(char byte_to_send);

bool previous_light_state = false;
bool current_light_state = false;
bool received_all_character = false;

/* function prototypes */
bool read_LDR_as_digital();
char read_byte_from_light();
void print_character(char c);

void setup() {
  pinMode(LDR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  message_length = strlen(received_buffer);

  Serial.begin(9600);
  Serial.println("Repeater node started.");
}

void loop() {
  current_light_state = read_LDR_as_digital();

  // Detect falling edge: HIGH → LOW = start bit detected
  if (previous_light_state == true && current_light_state == false) {
    // if (buffer_index < MAX_BYTES) {
    char received_character = read_byte_from_light();
    // received_buffer[buffer_index++] = received_character;
    print_character(received_character);
    //}

    // received_all_character = true;
  }

  previous_light_state = current_light_state;

  //   if (received_all_character == true) {
  //     for (int i = 0; i < message_length; i++) {
  //       char outgoing_char = received_buffer[i];

  //       // Print the character being transmitted for debugging
  //       Serial.print("Transmitting char: ");
  //       Serial.print(outgoing_char);
  //       Serial.print("   ASCII: ");
  //       Serial.println((int)outgoing_char);

  //       send_byte(outgoing_char);
  //     }

  //     received_all_character = false;
  //   }

  // delay(1000);
}

bool read_LDR_as_digital() {
  int light_value = analogRead(LDR_PIN);
  // Serial.printf("light value: %d\n", light_value);
  return (light_value > LIGHT_THRESHOLD);
}

char read_byte_from_light() {
  char reconstructed_byte = 0;

  // align sampling to middle of first data bit
  delay(PERIOD * 1.5);

  for (int bit_index = 0; bit_index < 8; bit_index++) {
    bool bit_value = read_LDR_as_digital(); // returns 0 or 1

    // shift bit into correct position
    reconstructed_byte |= (bit_value << bit_index);

    // wait for next bit slot
    delay(PERIOD);
  }

  return reconstructed_byte;
}

void print_character(char c) { Serial.print(c); }

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