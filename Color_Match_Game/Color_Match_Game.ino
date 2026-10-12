/*
  COLOR MATCH GAME  for Arduino MEGA 2560
  An outreach activity for kids about how screens mix colors from Red, Green and Blue light.

  HOW TO PLAY
    LED 1 (SECRET)  shows a secret color.
    LED 2 (YOURS)   shows the color you mix with the three knobs:
                    knob 1 is Red, knob 2 is Green, knob 3 is Blue.
    LED 3 (METER)   stays off until your color is close enough,
                    then it beeps once and pulses red to say "you found it, press the button now".
    When the meter pulses, press the BUTTON.
      Match:      a random victory word and song, sparkly rainbow party, score goes up,
                  new secret color.
      Not yet:    "OOPS" and a silly sound, keep trying.
    Every 5 matches plays a bigger party.

  LEVELS
    Level 1 (score 0 to 4):  only primary and secondary colors
                             (red, green, blue, yellow, cyan, magenta)
    Level 2 (score 5 to 9):  same colors, but you have to be a little closer
    Level 3 (score 10 up):   every color, including in between colors like orange and pink
    A quick power up sound plays when a new level starts.
    If nobody plays for a while, the game goes back to rainbow mode for the next player.
    Press the button or turn a knob to start.

  SCORE DISPLAY (4 digit, 7 segment)
    Rainbow mode:  spinning animation, "PLAY", and the high score ("HI" and a number)
    Playing:       your score
    Match:         a random word: YAY, YES, GOOd, nICE, EPIC, ACE, PrO, SUPr
                   and "COOL" every 5 matches
    Not yet:       "OOPS"

  STARTUP CHECK
    Every time it powers on, the display shows 8888 and each LED flashes red, green,
    then blue, with a beep. A missing color means a wiring problem on that leg.
    Open Serial Monitor at 9600 baud to see the knob readings and the secret color.

  WIRING
    LED 1 (SECRET)  red leg pin 2,  green leg pin 3,  blue leg pin 4
    LED 2 (YOURS)   red leg pin 5,  green leg pin 6,  blue leg pin 7
    LED 3 (METER)   red leg pin 8,  green leg pin 11, blue leg pin 12
      Every color leg has its own 330 ohm resistor.
      Every long (common) leg goes to the GND rail.
    Knobs           middle leg to A0 (Red), A1 (Green), A2 (Blue)
                    outer legs to the 5V rail and the GND rail
    Button          one leg to pin 22, the diagonally opposite leg to the GND rail
    Buzzer          passive buzzer, plus leg to pin 24, other leg to the GND rail
    Display 5461AS  wired in order around the double header at the end of the Mega.
                    Bottom row of the display goes along the EVEN pins,
                    top row comes back along the ODD pins, so each display
                    column lands on one side by side pair of Mega pins:

                      Display pin:  1   2   3   4   5   6      (bottom row, left to right)
                      Mega pin:    42  44  46  48  50  52

                      Display pin: 12  11  10   9   8   7      (top row, left to right)
                      Mega pin:    43  45  47  49  51  53

                    Pins 1, 2, 3, 4, 5, 7, 10, 11 are segments: put a 1k resistor in each.
                    Pins 6, 8, 9, 12 are digits: wire them straight, no resistor.
                    Pin 3 is the decimal point; it is never lit, so that wire is optional.
*/

// Settings
const bool COMMON_ANODE = false;       // true only if the long LED leg goes to 5V instead of GND
const bool SEG_COMMON_ANODE = false;   // display: 5461AS is common cathode; set true if the display looks inverted
// How close counts as a match at each level (0 to 255, bigger is easier)
const int  TOLERANCE_LEVEL1 = 110;     // score 0 to 4
const int  TOLERANCE_LEVEL2 = 85;      // score 5 to 9, a little harder
const int  TOLERANCE_LEVEL3 = 110;     // score 10 and up, harder colors so a bit more room
const unsigned long IDLE_MS = 45000;   // back to rainbow mode after this long with nobody playing
const float GAMMA = 2.2;               // makes knob brightness look even to your eyes
const uint8_t METER_PULSE[3] = { 255, 0, 0 };   // meter pulse color (R, G, B), red

// Pins
const uint8_t LED_PINS[3][3] = {
  { 2, 3, 4 },     // LED 1 SECRET: R, G, B
  { 5, 6, 7 },     // LED 2 YOURS:  R, G, B
  { 8, 11, 12 }    // LED 3 METER:  R, G, B
};
const uint8_t SECRET = 0, YOURS = 1, METER = 2;

const uint8_t POT_PINS[3] = { A0, A1, A2 };
const uint8_t BUTTON_PIN = 22;
const uint8_t BUZZER_PIN = 24;

// Which Mega pin each display pin is wired to (display pins 1 to 12, index 0 unused)
const uint8_t MEGA_PIN_FOR[13] = { 0, 42, 44, 46, 48, 50, 52, 53, 51, 49, 47, 45, 43 };

// Display pin numbers for each segment and digit on the 5461AS
const uint8_t SEG_PINS[8] = {            // segments A B C D E F G, then the decimal point
  MEGA_PIN_FOR[11], MEGA_PIN_FOR[7], MEGA_PIN_FOR[4], MEGA_PIN_FOR[2],
  MEGA_PIN_FOR[1],  MEGA_PIN_FOR[10], MEGA_PIN_FOR[5], MEGA_PIN_FOR[3]
};
const uint8_t DIGIT_PINS[4] = {          // digits 1 to 4, left to right
  MEGA_PIN_FOR[12], MEGA_PIN_FOR[9], MEGA_PIN_FOR[8], MEGA_PIN_FOR[6]
};

// Secret colors. The first six are the primary and secondary colors (levels 1 and 2).
// White and the rest join at level 3; the rest use one color at half strength,
// so kids learn that knobs can go halfway too.
const uint8_t NUM_TARGETS = 13;
const uint8_t NUM_EASY_TARGETS = 6;
const uint8_t TARGETS[NUM_TARGETS][3] = {
  { 255,   0,   0 },   // Red
  {   0, 255,   0 },   // Green
  {   0,   0, 255 },   // Blue
  { 255, 255,   0 },   // Yellow     (red and green)
  {   0, 255, 255 },   // Cyan       (green and blue)
  { 255,   0, 255 },   // Magenta    (red and blue)
  { 255, 255, 255 },   // White      (all three)
  { 255, 128,   0 },   // Orange     (red and half green)
  { 128, 255,   0 },   // Lime       (green and half red)
  { 255,   0, 128 },   // Pink       (red and half blue)
  { 128,   0, 255 },   // Purple     (blue and half red)
  {   0, 128, 255 },   // Sky Blue   (blue and half green)
  {   0, 255, 128 }    // Mint       (green and half blue)
};
const char* TARGET_NAMES[NUM_TARGETS] = {
  "Red", "Green", "Blue", "Yellow", "Cyan", "Magenta", "White",
  "Orange", "Lime", "Pink", "Purple", "Sky Blue", "Mint"
};

// Musical notes (Hz)
const int NOTE_C4 = 262, NOTE_E4 = 330, NOTE_G4 = 392,
          NOTE_C5 = 523, NOTE_D5 = 587, NOTE_E5 = 659, NOTE_F5 = 698, NOTE_G5 = 784, NOTE_A5 = 880,
          NOTE_C6 = 1047, NOTE_D6 = 1175, NOTE_E6 = 1319, NOTE_G6 = 1568;

// Victory songs, one is picked at random for each match (0 marks the end of a song)
const int VICTORY_SONGS[5][7] = {
  { NOTE_C5, NOTE_E5, NOTE_G5, NOTE_C6, 0 },
  { NOTE_G5, NOTE_E5, NOTE_G5, NOTE_C6, 0 },
  { NOTE_C5, NOTE_D5, NOTE_E5, NOTE_G5, NOTE_E5, NOTE_C6, 0 },
  { NOTE_E5, NOTE_G5, NOTE_A5, NOTE_G5, NOTE_C6, 0 },
  { NOTE_A5, NOTE_G5, NOTE_E5, NOTE_G5, NOTE_C6, 0 }
};

// Sparkle notes that twinkle during the rainbow party
const int SPARKLE_NOTES[6] = { NOTE_C5, NOTE_D5, NOTE_E5, NOTE_G5, NOTE_A5, NOTE_C6 };

// Display letters and numbers (bit 0 is segment A, bit 6 is segment G)
const uint8_t SEG_NUM[10] = { 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F };
const uint8_t L_A = 0x77, L_C = 0x39, L_H = 0x76, L_I = 0x06, L_L = 0x38, L_O = 0x3F,
              L_P = 0x73, L_S = 0x6D, L_Y = 0x6E, L_r = 0x50, L_d = 0x5E, L_E = 0x79,
              L_G = 0x3D, L_n = 0x54, L_U = 0x3E, L_BLANK = 0x00;

// Victory words, one is picked at random for each match
const uint8_t NUM_PHRASES = 8;
const uint8_t PHRASES[NUM_PHRASES][4] = {
  { L_Y, L_A, L_Y, L_BLANK },   // YAY
  { L_Y, L_E, L_S, L_BLANK },   // YES
  { L_G, L_O, L_O, L_d },       // GOOd
  { L_n, L_I, L_C, L_E },       // nICE
  { L_E, L_P, L_I, L_C },       // EPIC
  { L_A, L_C, L_E, L_BLANK },   // ACE
  { L_P, L_r, L_O, L_BLANK },   // PrO
  { L_S, L_U, L_P, L_r }        // SUPr
};

// State
enum GameState : uint8_t { ATTRACT, PLAYING };
GameState state = ATTRACT;

int8_t targetIndex = -1;
uint8_t player[3];            // the color mixed with the knobs, 0 to 255 each
float potSmooth[3];
int lastActivityPot[3];
unsigned long lastActivity = 0;
unsigned int score = 0;
unsigned int bestScore = 0;

bool buttonReading = HIGH, buttonStable = HIGH;
unsigned long buttonChangedAt = 0;
unsigned long lastSerial = 0;

bool wasClose = false;                 // was the color close enough last time we checked
unsigned long lastFoundBeep = 0;
int8_t lastPhrase = -1;

// Display buffer, refreshed in the background by a timer interrupt
volatile uint8_t segBuf[4] = { 0, 0, 0, 0 };
volatile uint8_t curDigit = 0;

// Display helpers
void showRaw(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
  segBuf[0] = a; segBuf[1] = b; segBuf[2] = c; segBuf[3] = d;
}

void showNumber(unsigned int n) {       // right aligned, no leading zeros
  if (n > 9999) n = 9999;
  for (int8_t i = 3; i >= 0; i--) {
    if (n == 0 && i < 3) segBuf[i] = L_BLANK;
    else segBuf[i] = SEG_NUM[n % 10];
    n /= 10;
  }
}

void showHighScore() {                   // "HI" then the best score, like HI 7 or HI12
  unsigned int n = bestScore > 99 ? 99 : bestScore;
  segBuf[0] = L_H;
  segBuf[1] = L_I;
  segBuf[2] = (n >= 10) ? SEG_NUM[n / 10] : L_BLANK;
  segBuf[3] = SEG_NUM[n % 10];
}

// Spinning snake around the outside edge of the display
const uint8_t SNAKE_DIGIT[12] = { 0, 1, 2, 3, 3, 3, 3, 2, 1, 0, 0, 0 };
const uint8_t SNAKE_SEG[12]   = { 0x01, 0x01, 0x01, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08, 0x08, 0x10, 0x20 };

void showSnake(uint8_t step) {
  uint8_t buf[4] = { 0, 0, 0, 0 };
  for (uint8_t t = 0; t < 4; t++) {      // 4 lit segments make the snake body
    uint8_t s = (step + 12 - t) % 12;
    buf[SNAKE_DIGIT[s]] |= SNAKE_SEG[s];
  }
  showRaw(buf[0], buf[1], buf[2], buf[3]);
}

// Timer 5 interrupt: lights one digit at a time, 1000 times per second,
// so the display keeps working even while songs and animations play.
ISR(TIMER5_COMPA_vect) {
  const uint8_t digitOff = SEG_COMMON_ANODE ? LOW : HIGH;
  const uint8_t digitOn  = SEG_COMMON_ANODE ? HIGH : LOW;
  digitalWrite(DIGIT_PINS[curDigit], digitOff);
  curDigit = (curDigit + 1) & 3;
  uint8_t s = segBuf[curDigit];
  for (uint8_t i = 0; i < 8; i++) {
    bool lit = (i < 7) && ((s >> i) & 1);  // decimal point always off
    digitalWrite(SEG_PINS[i], (lit != SEG_COMMON_ANODE) ? HIGH : LOW);
  }
  digitalWrite(DIGIT_PINS[curDigit], digitOn);
}

void startDisplayTimer() {
  noInterrupts();
  TCCR5A = 0;
  TCCR5B = (1 << WGM52) | (1 << CS51) | (1 << CS50);   // CTC mode, clock divided by 64
  OCR5A = 249;                                         // 16 MHz / 64 / 250 gives 1000 per second
  TIMSK5 |= (1 << OCIE5A);
  interrupts();
}

// LED helpers
uint8_t gammaFix(uint8_t v) {
  return (uint8_t)(pow(v / 255.0, GAMMA) * 255.0 + 0.5);
}

void setLed(uint8_t led, uint8_t r, uint8_t g, uint8_t b) {
  uint8_t vals[3] = { gammaFix(r), gammaFix(g), gammaFix(b) };
  for (uint8_t c = 0; c < 3; c++) {
    analogWrite(LED_PINS[led][c], COMMON_ANODE ? 255 - vals[c] : vals[c]);
  }
}

void allLeds(uint8_t r, uint8_t g, uint8_t b) {
  for (uint8_t led = 0; led < 3; led++) setLed(led, r, g, b);
}

// Rainbow color from a position 0 to 255
void wheel(uint8_t pos, uint8_t &r, uint8_t &g, uint8_t &b) {
  if (pos < 85) {
    r = 255 - pos * 3; g = pos * 3; b = 0;
  } else if (pos < 170) {
    pos -= 85;
    r = 0; g = 255 - pos * 3; b = pos * 3;
  } else {
    pos -= 170;
    r = pos * 3; g = 0; b = 255 - pos * 3;
  }
}

// Sound helper
void note(int freq, int ms) {
  tone(BUZZER_PIN, freq, ms);
  delay(ms + ms / 5);
}

// Knobs
void readPots() {
  for (uint8_t i = 0; i < 3; i++) {
    int raw = analogRead(POT_PINS[i]);
    potSmooth[i] += (raw - potSmooth[i]) * 0.25f;
    // small dead zone at both ends so fully off and fully on are easy to reach
    float x = (potSmooth[i] - 20.0f) / (1003.0f - 20.0f);
    player[i] = (uint8_t)(constrain(x, 0.0f, 1.0f) * 255.0f + 0.5f);
  }
}

bool knobsMoved() {
  bool moved = false;
  for (uint8_t i = 0; i < 3; i++) {
    int now = (int)potSmooth[i];
    if (abs(now - lastActivityPot[i]) > 60) {
      moved = true;
      lastActivityPot[i] = now;
    }
  }
  return moved;
}

// Button (returns true once per press)
bool buttonPressed() {
  bool reading = digitalRead(BUTTON_PIN);
  if (reading != buttonReading) {
    buttonReading = reading;
    buttonChangedAt = millis();
  }
  if (millis() - buttonChangedAt > 30 && reading != buttonStable) {
    buttonStable = reading;
    if (buttonStable == LOW) return true;
  }
  return false;
}

// Game logic
int biggestDifference() {
  int worst = 0;
  for (uint8_t c = 0; c < 3; c++) {
    int d = abs((int)player[c] - (int)TARGETS[targetIndex][c]);
    if (d > worst) worst = d;
  }
  return worst;
}

uint8_t currentLevel() {
  if (score < 5)  return 1;
  if (score < 10) return 2;
  return 3;
}

int currentTolerance() {
  switch (currentLevel()) {
    case 1:  return TOLERANCE_LEVEL1;
    case 2:  return TOLERANCE_LEVEL2;
    default: return TOLERANCE_LEVEL3;
  }
}

bool closeEnough() {
  return biggestDifference() <= currentTolerance();
}

void newTarget() {
  uint8_t poolSize = (currentLevel() < 3) ? NUM_EASY_TARGETS : NUM_TARGETS;
  int8_t next;
  do {
    next = random(poolSize);
  } while (next == targetIndex);
  targetIndex = next;
  wasClose = false;
  Serial.print("New secret color: ");
  Serial.println(TARGET_NAMES[targetIndex]);
}

void startGame() {
  state = PLAYING;
  score = 0;
  lastActivity = millis();
  showRaw(L_P, L_L, L_A, L_Y);           // PLAY
  note(NOTE_C5, 80);
  note(NOTE_G5, 120);
  delay(400);
  showNumber(score);
  newTarget();
}

void showMeter() {
  // Off unless the color is close enough, then a smooth red pulse says "press the button now"
  bool close = closeEnough();
  if (close) {
    // Beep once when the color is first found (not again for a moment, so it does not chatter)
    if (!wasClose && millis() - lastFoundBeep > 800) {
      tone(BUZZER_PIN, NOTE_E6, 60);
      delay(80);
      tone(BUZZER_PIN, NOTE_G6, 90);
      lastFoundBeep = millis();
    }
    float wave = 0.5f + 0.5f * sin(millis() / 120.0f);     // 0 to 1, about 1.3 pulses per second
    float level = 0.35f + 0.65f * wave;                      // never fully dark while pulsing
    setLed(METER, (uint8_t)(METER_PULSE[0] * level),
                  (uint8_t)(METER_PULSE[1] * level),
                  (uint8_t)(METER_PULSE[2] * level));
  } else {
    setLed(METER, 0, 0, 0);
  }
  wasClose = close;
}

void rainbowParty(unsigned long ms) {
  // rainbow on all LEDs with random twinkling notes
  unsigned long start = millis();
  uint8_t pos = 0;
  uint8_t frame = 0;
  while (millis() - start < ms) {
    for (uint8_t led = 0; led < 3; led++) {
      uint8_t r, g, b;
      wheel(pos + led * 85, r, g, b);
      setLed(led, r, g, b);
    }
    if (frame % 6 == 0) tone(BUZZER_PIN, SPARKLE_NOTES[random(6)], 50);
    frame++;
    pos += 4;
    delay(15);
  }
  noTone(BUZZER_PIN);
}

void showVictoryWord() {
  int8_t p;
  do {
    p = random(NUM_PHRASES);
  } while (p == lastPhrase);
  lastPhrase = p;
  showRaw(PHRASES[p][0], PHRASES[p][1], PHRASES[p][2], PHRASES[p][3]);
}

void celebrate() {
  score++;
  if (score > bestScore) bestScore = score;
  Serial.print("MATCH! Score: ");
  Serial.println(score);

  bool bigParty = (score % 5 == 0);
  if (bigParty) showRaw(L_C, L_O, L_O, L_L);        // COOL
  else          showVictoryWord();

  // random victory song with the target color flashing on every note
  const int *song = VICTORY_SONGS[random(5)];
  const uint8_t *t = TARGETS[targetIndex];
  for (uint8_t i = 0; song[i] != 0; i++) {
    allLeds(t[0], t[1], t[2]);
    note(song[i], song[i + 1] == 0 ? 220 : 100);    // last note held longer
    allLeds(0, 0, 0);
    delay(25);
  }

  if (bigParty) {
    // bugle call fanfare, then a long sparkly party
    note(NOTE_G4, 110); note(NOTE_C5, 110); note(NOTE_E5, 110);
    note(NOTE_G5, 250); note(NOTE_E5, 110); note(NOTE_G5, 450);
    rainbowParty(3500);
  } else {
    rainbowParty(1400);
  }

  // New level starts at score 5 and score 10: quick power up sweep
  if (score == 5 || score == 10) {
    Serial.print("LEVEL UP! Now level ");
    Serial.println(currentLevel());
    for (int f = 400; f < 1600; f += 60) {
      tone(BUZZER_PIN, f, 20);
      delay(18);
    }
    noTone(BUZZER_PIN);
  }

  showNumber(score);
  newTarget();
  lastActivity = millis();
}

void tryAgain() {
  Serial.println("Not yet, keep trying");
  showRaw(L_O, L_O, L_P, L_S);           // OOPS
  setLed(METER, 0, 0, 0);
  // sliding "womp womp" sound
  for (uint8_t w = 0; w < 2; w++) {
    for (int f = 400; f > 250; f -= 10) {
      tone(BUZZER_PIN, f, 15);
      delay(12);
    }
    noTone(BUZZER_PIN);
    delay(80);
  }
  note(NOTE_C4, 300);
  showNumber(score);
  lastActivity = millis();
}

void runAttract() {
  // slow rainbow on all three LEDs, each one a different color
  uint8_t pos = (millis() / 20) & 0xFF;
  for (uint8_t led = 0; led < 3; led++) {
    uint8_t r, g, b;
    wheel(pos + led * 85, r, g, b);
    setLed(led, r, g, b);
  }

  // display loop: snake for 3 s, PLAY for 1.5 s, high score for 1.5 s
  unsigned long t = millis() % 6000;
  if (t < 3000)                         showSnake((millis() / 70) % 12);
  else if (t < 4500)                    showRaw(L_P, L_L, L_A, L_Y);
  else if (bestScore > 0)               showHighScore();
  else                                  showSnake((millis() / 70) % 12);
}

void startupCheck() {
  Serial.println("Startup check: display shows 8888, each LED flashes red, green, blue");
  showRaw(0x7F, 0x7F, 0x7F, 0x7F);      // 8888 lights every segment
  const int beeps[3] = { NOTE_C5, NOTE_E5, NOTE_G5 };
  for (uint8_t led = 0; led < 3; led++) {
    tone(BUZZER_PIN, beeps[led], 80);
    for (uint8_t c = 0; c < 3; c++) {
      uint8_t rgb[3] = { 0, 0, 0 };
      rgb[c] = 255;
      setLed(led, rgb[0], rgb[1], rgb[2]);
      delay(250);
    }
    setLed(led, 0, 0, 0);
  }
  showRaw(0, 0, 0, 0);
}

void printStatus() {
  if (millis() - lastSerial < 500) return;
  lastSerial = millis();
  Serial.print("Knobs R G B: ");
  Serial.print(player[0]); Serial.print(' ');
  Serial.print(player[1]); Serial.print(' ');
  Serial.print(player[2]);
  if (state == PLAYING) {
    Serial.print("   Secret: ");
    Serial.print(TARGET_NAMES[targetIndex]);
    Serial.print("   Off by: ");
    Serial.print(biggestDifference());
    Serial.print("   Score: ");
    Serial.print(score);
    Serial.print("   Level: ");
    Serial.print(currentLevel());
  } else {
    Serial.print("   (rainbow mode, press button to play)");
  }
  Serial.println();
}

// Setup and Loop
void setup() {
  Serial.begin(9600);
  for (uint8_t led = 0; led < 3; led++)
    for (uint8_t c = 0; c < 3; c++)
      pinMode(LED_PINS[led][c], OUTPUT);
  allLeds(0, 0, 0);

  for (uint8_t i = 0; i < 8; i++) pinMode(SEG_PINS[i], OUTPUT);
  for (uint8_t i = 0; i < 4; i++) {
    pinMode(DIGIT_PINS[i], OUTPUT);
    digitalWrite(DIGIT_PINS[i], SEG_COMMON_ANODE ? LOW : HIGH);   // all digits off
  }
  startDisplayTimer();

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);

  for (uint8_t i = 0; i < 3; i++) {
    potSmooth[i] = analogRead(POT_PINS[i]);
    lastActivityPot[i] = (int)potSmooth[i];
  }
  randomSeed(analogRead(A5) ^ micros());

  startupCheck();
  Serial.println("Ready! Press the button to play.");
}

void loop() {
  readPots();
  bool pressed = buttonPressed();
  bool moved = knobsMoved();

  if (state == ATTRACT) {
    runAttract();
    if (pressed || moved) startGame();
  } else {
    if (moved) lastActivity = millis();

    const uint8_t *t = TARGETS[targetIndex];
    setLed(SECRET, t[0], t[1], t[2]);
    setLed(YOURS, player[0], player[1], player[2]);
    showMeter();

    if (pressed) {
      if (closeEnough()) celebrate();
      else tryAgain();
    }

    if (millis() - lastActivity > IDLE_MS) {
      state = ATTRACT;
      Serial.println("Nobody playing, back to rainbow mode");
    }
  }

  printStatus();
}
