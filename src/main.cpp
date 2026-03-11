#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>

// Display pins
static constexpr int TFT_SCLK = 18;
static constexpr int TFT_MOSI = 23;
static constexpr int TFT_CS   = 16;
static constexpr int TFT_DC   = 4;
static constexpr int TFT_RST  = 17;

// Touch pins
static constexpr int TOUCH_SLOW = 33;
static constexpr int TOUCH_MODE = 32;
static constexpr int TOUCH_FAST = 27;

// Lower value means "more touched". These usually need tuning.
static constexpr int TOUCH_THRESHOLD_SLOW = 30;
static constexpr int TOUCH_THRESHOLD_MODE = 30;
static constexpr int TOUCH_THRESHOLD_FAST = 30;

// Display size
static constexpr int SCREEN_W = 240;
static constexpr int SCREEN_H = 240;
static constexpr int CENTER_X = SCREEN_W / 2;
static constexpr int CENTER_Y = SCREEN_H / 2;
static constexpr int RADIUS   = 118;

// Particle settings
static constexpr int PARTICLE_COUNT = 48;
static constexpr float MIN_SPEED = 0.0025f;
static constexpr float MAX_SPEED = 0.0800f;
static constexpr float SPEED_STEP = 0.0025f;

// Debounce / repeat timings
static constexpr uint32_t TOUCH_REPEAT_MS = 180;
static constexpr uint32_t MODE_REPEAT_MS  = 250;
static constexpr uint32_t FRAME_MS        = 10;   // about 100 FPS cap

Adafruit_GC9A01A tft(TFT_CS, TFT_DC, TFT_RST);

struct Particle {
  float angle;
  float radius;
  float radialSpeed;
  float tangentialBias;
  uint16_t color;
  uint8_t size;
};

Particle particles[PARTICLE_COUNT];

float rotationAngle = 0.0f;
float rotationSpeed = 0.0125f;
uint8_t drawMode = 0;
uint8_t lastRenderedMode = 0;
bool hasPreviousFrame = false;

uint32_t lastTouchSlowMs = 0;
uint32_t lastTouchFastMs = 0;
uint32_t lastTouchModeMs = 0;
uint32_t lastFrameMs = 0;

uint16_t makeColor565(uint8_t r, uint8_t g, uint8_t b) {
  return tft.color565(r, g, b);
}

uint16_t randomBrightColor() {
  uint8_t r = random(80, 256);
  uint8_t g = random(80, 256);
  uint8_t b = random(80, 256);
  return makeColor565(r, g, b);
}

float randf(float minValue, float maxValue) {
  return minValue + (maxValue - minValue) * (float(random(0, 10000)) / 9999.0f);
}

void resetParticle(Particle &p) {
  p.angle = randf(0.0f, 2.0f * PI);
  p.radius = randf(8.0f, float(RADIUS));
  p.radialSpeed = randf(-0.35f, 0.35f);
  p.tangentialBias = randf(-0.02f, 0.02f);
  p.color = randomBrightColor();
  p.size = uint8_t(random(1, 4));
}

void initParticles() {
  for (int i = 0; i < PARTICLE_COUNT; ++i) {
    resetParticle(particles[i]);
  }
}

void setupSpiPins() {
  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
}

void setupDisplay() {
  setupSpiPins();
  tft.begin();
  tft.setRotation(0);
  tft.fillScreen(GC9A01A_BLACK);
}

bool touchActive(int pin, int threshold) {
  return touchRead(pin) < threshold;
}

void updateTouchControls() {
  uint32_t now = millis();

  if (touchActive(TOUCH_FAST, TOUCH_THRESHOLD_FAST) && (now - lastTouchFastMs >= TOUCH_REPEAT_MS)) {
    rotationSpeed += SPEED_STEP;
    if (rotationSpeed > MAX_SPEED) rotationSpeed = MAX_SPEED;
    lastTouchFastMs = now;
  }

  if (touchActive(TOUCH_SLOW, TOUCH_THRESHOLD_SLOW) && (now - lastTouchSlowMs >= TOUCH_REPEAT_MS)) {
    rotationSpeed -= SPEED_STEP;
    if (rotationSpeed < MIN_SPEED) rotationSpeed = MIN_SPEED;
    lastTouchSlowMs = now;
  }

  if (touchActive(TOUCH_MODE, TOUCH_THRESHOLD_MODE) && (now - lastTouchModeMs >= MODE_REPEAT_MS)) {
    drawMode = (drawMode + 1) % 3;
    lastTouchModeMs = now;
  }
}

void updateParticleState(Particle &p) {
  // Slowly move particles inward or outward
  p.radius += p.radialSpeed;

  // Add angular movement based on current global rotation
  p.angle += rotationSpeed + p.tangentialBias;

  // Reflect at radial limits
  if (p.radius < 6.0f) {
    p.radius = 6.0f;
    p.radialSpeed = -p.radialSpeed;
  }

  if (p.radius > float(RADIUS)) {
    p.radius = float(RADIUS);
    p.radialSpeed = -p.radialSpeed;
  }

  // Small random drift
  p.tangentialBias += randf(-0.0008f, 0.0008f);
  if (p.tangentialBias > 0.03f) p.tangentialBias = 0.03f;
  if (p.tangentialBias < -0.03f) p.tangentialBias = -0.03f;
}

void drawModePixels(const Particle &p, float a, uint16_t color) {
  int x = int(CENTER_X + cosf(a) * p.radius);
  int y = int(CENTER_Y + sinf(a) * p.radius);
  if (x >= 0 && x < SCREEN_W && y >= 0 && y < SCREEN_H) {
    tft.drawPixel(x, y, color);
  }
}

void drawModeDots(const Particle &p, float a, uint16_t color) {
  int x = int(CENTER_X + cosf(a) * p.radius);
  int y = int(CENTER_Y + sinf(a) * p.radius);
  if (x >= 0 && x < SCREEN_W && y >= 0 && y < SCREEN_H) {
    tft.fillCircle(x, y, p.size, color);
  }
}

void drawModeLines(const Particle &p, float a, uint16_t color) {
  int x = int(CENTER_X + cosf(a) * p.radius);
  int y = int(CENTER_Y + sinf(a) * p.radius);
  tft.drawLine(CENTER_X, CENTER_Y, x, y, color);
}

void renderNextFrame(bool eraseOld) {
  // Six-fold radial replication. This is useful even without external mirrors.
  static constexpr int SYMMETRY = 6;
  static constexpr float STEP = (2.0f * PI) / float(SYMMETRY);
  const float oldRotation = rotationAngle;
  float newRotation = rotationAngle + rotationSpeed;
  if (newRotation > 2.0f * PI) {
    newRotation -= 2.0f * PI;
  }

  for (int i = 0; i < PARTICLE_COUNT; ++i) {
    Particle &p = particles[i];
    const Particle oldP = p;
    updateParticleState(p);

    for (int k = 0; k < SYMMETRY; ++k) {
      float oldA = oldP.angle + oldRotation + STEP * float(k);
      float newA = p.angle + newRotation + STEP * float(k);

      switch (drawMode) {
        case 0:
          if (eraseOld) {
            int oldX = int(CENTER_X + cosf(oldA) * oldP.radius);
            int oldY = int(CENTER_Y + sinf(oldA) * oldP.radius);
            int newX = int(CENTER_X + cosf(newA) * p.radius);
            int newY = int(CENTER_Y + sinf(newA) * p.radius);

            // Only clear pixels that are truly vacated.
            if (oldX != newX || oldY != newY) {
              drawModePixels(oldP, oldA, GC9A01A_BLACK);
            }
          }
          drawModePixels(p, newA, p.color);
          break;
        case 1:
          if (eraseOld) drawModeDots(oldP, oldA, GC9A01A_BLACK);
          drawModeDots(p, newA, p.color);
          break;
        case 2:
          if (eraseOld) drawModeLines(oldP, oldA, GC9A01A_BLACK);
          drawModeLines(p, newA, p.color);
          break;
      }
    }
  }

  rotationAngle = newRotation;
}

void printStatus() {
  static uint32_t lastPrint = 0;
  uint32_t now = millis();
  if (now - lastPrint >= 1000) {
    Serial.print("speed=");
    Serial.print(rotationSpeed, 4);
    Serial.print(" mode=");
    Serial.print(drawMode);
    Serial.print(" touch(slow/mode/fast)=");
    Serial.print(touchRead(TOUCH_SLOW));
    Serial.print("/");
    Serial.print(touchRead(TOUCH_MODE));
    Serial.print("/");
    Serial.println(touchRead(TOUCH_FAST));
    lastPrint = now;
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);

  randomSeed(uint32_t(esp_random()));

  setupDisplay();
  initParticles();

  Serial.println("GC9A01 kaleidoscope start");
}

void loop() {
  uint32_t now = millis();

  updateTouchControls();

  if (now - lastFrameMs >= FRAME_MS) {
    lastFrameMs = now;

    if (drawMode != lastRenderedMode) {
      tft.fillScreen(GC9A01A_BLACK);
      lastRenderedMode = drawMode;
      hasPreviousFrame = false;
    }

    renderNextFrame(hasPreviousFrame);
    hasPreviousFrame = true;
  }

  printStatus();
}
