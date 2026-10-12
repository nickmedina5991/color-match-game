/*
  Wiring Test for the Color Match Game on Arduino MEGA 2560
  Checks the buzzer, all 9 LED channels, the 3 knobs, and the button (not the display).

  Upload this, then open Tools > Serial Monitor and set it to 9600 baud.

  Part 1  Buzzer: beeps for one second at startup.
  Part 2  LEDs: lights ONE colour of ONE LED at a time, full brightness,
          for 1.5 seconds each. The Serial Monitor says what should be lit.
          Write down anything that does not match.
  Part 3  Pots and button: prints the three pot readings and the button
          state 5 times per second, forever.
          Each pot should read about 0 at one end and about 1023 at the other.
          The button should say PRESSED only while you hold it.
*/

const uint8_t LED_PINS[3][3] = {
  { 2, 3, 4 },     // LED 1: R, G, B
  { 5, 6, 7 },     // LED 2: R, G, B
  { 8, 11, 12 }    // LED 3: R, G, B
};
const uint8_t POT_PINS[3] = { A0, A1, A2 };
const uint8_t BUTTON_PIN = 22;
const uint8_t BUZZER_PIN = 24;

const char* COLOR_NAMES[3] = { "RED", "GREEN", "BLUE" };

void allOff() {
  for (uint8_t led = 0; led < 3; led++)
    for (uint8_t c = 0; c < 3; c++)
      digitalWrite(LED_PINS[led][c], LOW);
}

void setup() {
  Serial.begin(9600);
  delay(500);

  for (uint8_t led = 0; led < 3; led++)
    for (uint8_t c = 0; c < 3; c++)
      pinMode(LED_PINS[led][c], OUTPUT);
  allOff();

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);

  // Part 1: buzzer
  Serial.println();
  Serial.println("PART 1: buzzer should beep for 1 second now");
  tone(BUZZER_PIN, 1000, 1000);
  delay(1500);

  // Part 2: each LED colour on its own
  Serial.println();
  Serial.println("PART 2: LED test, only ONE colour on ONE LED should light at a time");
  for (uint8_t led = 0; led < 3; led++) {
    for (uint8_t c = 0; c < 3; c++) {
      allOff();
      Serial.print("  LED ");
      Serial.print(led + 1);
      Serial.print(" should be ");
      Serial.print(COLOR_NAMES[c]);
      Serial.print("   (pin ");
      Serial.print(LED_PINS[led][c]);
      Serial.println(")");
      digitalWrite(LED_PINS[led][c], HIGH);
      delay(1500);
    }
  }
  allOff();

  Serial.println("  All three LEDs should now be WHITE");
  for (uint8_t led = 0; led < 3; led++)
    for (uint8_t c = 0; c < 3; c++)
      digitalWrite(LED_PINS[led][c], HIGH);
  delay(2000);
  allOff();

  // Part 3 runs in loop()
  Serial.println();
  Serial.println("PART 3: turn each pot fully both ways, and press the button");
  Serial.println("  Pot1(A0)  Pot2(A1)  Pot3(A2)  Button");
}

void loop() {
  for (uint8_t i = 0; i < 3; i++) {
    int v = analogRead(POT_PINS[i]);
    Serial.print("  ");
    if (v < 1000) Serial.print(' ');
    if (v < 100)  Serial.print(' ');
    if (v < 10)   Serial.print(' ');
    Serial.print(v);
    Serial.print("    ");
  }
  Serial.println(digitalRead(BUTTON_PIN) == LOW ? "PRESSED" : "up");
  delay(200);
}
