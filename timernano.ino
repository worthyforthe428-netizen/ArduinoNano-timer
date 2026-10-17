#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define SCREEN_W 128
#define SCREEN_H 64
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_W, SCREEN_H, &Wire, -1);

const int BTN_START = 2;   // start / pause
const int BTN_SET   = 3;   // add 1 minute (when paused)
const int BUZZER    = 9;

unsigned long durationMs = 5UL * 60UL * 1000UL;
unsigned long remainingMs = durationMs;
unsigned long lastTick = 0;
bool running = false;
bool finished = false;

bool pressed(int pin) {
  static unsigned long lastPress[20];
  if (digitalRead(pin) == LOW && millis() - lastPress[pin] > 250) {
    lastPress[pin] = millis();
    return true;
  }
  return false;
}

void draw() {
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);

  unsigned long totalSec = (remainingMs + 999) / 1000;
  char buf[8];
  sprintf(buf, "%02d:%02d", (int)(totalSec / 60), (int)(totalSec % 60));

  display.setTextSize(4);
  display.setCursor(4, 14);
  display.print(buf);

  display.setTextSize(1);
  display.setCursor(0, 56);
  if (finished)     display.print(F("TIME'S UP!"));
  else if (running) display.print(F("Running"));
  else              display.print(F("Paused  SET:+1min"));

  display.display();
}

void setup() {
  pinMode(BTN_START, INPUT_PULLUP);
  pinMode(BTN_SET, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);

  if (!display.begin(0x3C, true)) {   // try 0x3D if blank
    for (;;);
  }
  draw();
}

void loop() {
  if (pressed(BTN_START)) {
    if (finished) {
      finished = false;
      remainingMs = durationMs;
    } else {
      running = !running;
    }
    lastTick = millis();
  }

  if (pressed(BTN_SET) && !running) {
    durationMs += 60000UL;
    if (durationMs > 99UL * 60000UL) durationMs = 60000UL;
    remainingMs = durationMs;
    finished = false;
  }

  if (running) {
    unsigned long now = millis();
    unsigned long delta = now - lastTick;
    lastTick = now;
    if (delta >= remainingMs) {
      remainingMs = 0;
      running = false;
      finished = true;
    } else {
      remainingMs -= delta;
    }
  }

  digitalWrite(BUZZER, finished ? (millis() / 300) % 2 : LOW);

  static unsigned long lastDraw = 0;
  if (millis() - lastDraw > 100) {
    lastDraw = millis();
    draw();
  }
}