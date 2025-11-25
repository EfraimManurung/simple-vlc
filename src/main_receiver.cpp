/*
    Visible Light Communication (VLC) Receiver
    ------------------------------------------------
    Reads bits using an LDR (Light Dependent Resistor)
    and reconstructs bytes transmitted by the LED-based
    transmitter.

    TRANSMISSION FORMAT (per character):
        Start bit : LOW (LED OFF)
        Data bits : 8 bits, LSB first
        Stop bit  : HIGH (LED ON)

    RECEIVER OPERATION:
        1. Detect falling edge (HIGH → LOW) = start bit
        2. Wait 1.5 bit periods to center on data bit 0
        3. Sample 8 bits spaced exactly PERIOD ms apart
        4. Reconstruct the byte using bit shifting
        5. Print the ASCII character

    Example received bits for 'H'
        Transmitted (LSB first): 0 0 0 1 0 0 1 0
        Reconstructed (MSB view): 01001000 = 'H'

    Author: Efraim Manurung
    email: efraim.manurung@gmail.com

    v1.0.0
*/

#include <Arduino.h>

#define LDR_PIN 34    // Light sensor pin
#define THRESHOLD 100 // threshold that converts analog → digital
#define PERIOD 15     // bit duration (must match transmitter)

bool previous_light_state = false;
bool current_light_state = false;

/* function prototypes */
bool read_LDR_as_digital();
char read_byte_from_light();
void print_character(char c);

void setup() { Serial.begin(9600); }

/*
 ------------------------------------------------------------------------------
                                MAIN LOOP
------------------------------------------------------------------------------
    The receiver watches the LDR continuously.

    We detect the START BIT by watching for a falling edge:
         HIGH → LOW

    Explanation:
        Transmitted START bit is ALWAYS LOW.
        So if light was HIGH and suddenly becomes LOW,
        that means a new character is beginning.
------------------------------------------------------------------------------
*/
void loop() {
  current_light_state = read_LDR_as_digital();

  // Detect falling edge: HIGH → LOW = start bit detected
  if (previous_light_state == true && current_light_state == false) {
    char received_character = read_byte_from_light();
    print_character(received_character);
  }

  previous_light_state = current_light_state;
}

/*
 ------------------------------------------------------------------------------
                 CONVERT RAW ANALOG LDR DATA → DIGITAL 0/1
------------------------------------------------------------------------------
    The LDR produces an analog value 0–1023.

    THRESHOLD decides what counts as:

        Light ON  = 1  (voltage > THRESHOLD)
        Light OFF = 0  (voltage <= THRESHOLD)

    Must match transmitter brightness conditions.
------------------------------------------------------------------------------
*/
bool read_LDR_as_digital() {
  int light_value = analogRead(LDR_PIN);
  return (light_value > THRESHOLD);
}

/*
 ------------------------------------------------------------------------------
             READ AND RECONSTRUCT ONE FULL BYTE FROM LIGHT
------------------------------------------------------------------------------
    Timing is critical here.

    PROCESS:

    1. A falling edge tells us START BIT is happening.
    2. We wait 1.5 * bit_period to align with BIT 0 center.
       Why 1.5?
           - 1 period: finish the start bit
           - +0.5 period: move to center of data bit 0

    3. Sample 8 bits, one per PERIOD ms.
    4. Rebuild byte LSB-first:
            bit0 → shift 0
            bit1 → shift 1
            ...
            bit7 → shift 7

    FORMULA:
        result_byte = result_byte OR (bit_value << bit_index)
------------------------------------------------------------------------------
*/
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

/*
 ------------------------------------------------------------------------------
                       PRINT THE RECEIVED ASCII CHARACTER
------------------------------------------------------------------------------
*/
void print_character(char c) { Serial.print(c); }
