#include "kaleido.h"

namespace kaleido {

Adafruit_GC9A01A tft(TFT_CS, TFT_DC, TFT_RST);
GFXcanvas16 *frameCanvas = nullptr;
Particle particles[PARTICLE_COUNT];
BackgroundShard bgShards[BG_SHARD_COUNT];

float rotationAngle = 0.0f;
float rotationSpeed = 0.0125f;
uint8_t drawMode = 0;
uint8_t themeMode = 0; // 0=dark, 1=light, 2=daylight
uint8_t lastModeCycle = 255;

bool buttonLastRawPressed = false;
bool buttonStablePressed = false;
bool buttonLongHandled = false;
uint8_t pendingClicks = 0;
uint8_t speedPresetIndex = 1;
uint32_t buttonLastEdgeMs = 0;
uint32_t buttonPressedMs = 0;
uint32_t buttonReleasedMs = 0;
uint32_t lastFrameMs = 0;

uint16_t makeColor565(uint8_t r, uint8_t g, uint8_t b) {
  return tft.color565(r, g, b);
}

static uint16_t vividHsvColor565(float hDeg, float s, float v) {
  while (hDeg < 0.0f) hDeg += 360.0f;
  while (hDeg >= 360.0f) hDeg -= 360.0f;

  float c = v * s;
  float hh = hDeg / 60.0f;
  float x = c * (1.0f - fabsf(fmodf(hh, 2.0f) - 1.0f));
  float r1 = 0.0f;
  float g1 = 0.0f;
  float b1 = 0.0f;

  int region = int(hh);
  switch (region) {
    case 0: r1 = c; g1 = x; break;
    case 1: r1 = x; g1 = c; break;
    case 2: g1 = c; b1 = x; break;
    case 3: g1 = x; b1 = c; break;
    case 4: r1 = x; b1 = c; break;
    default: r1 = c; b1 = x; break;
  }

  float m = v - c;
  uint8_t r = uint8_t((r1 + m) * 255.0f);
  uint8_t g = uint8_t((g1 + m) * 255.0f);
  uint8_t b = uint8_t((b1 + m) * 255.0f);
  return makeColor565(r, g, b);
}

uint16_t themeFlatColor() {
  if (themeMode == 1) return GC9A01A_WHITE;
  if (themeMode == 2) return makeColor565(255, 242, 130);
  return GC9A01A_BLACK;
}

uint16_t backgroundColor() {
  return themeFlatColor();
}

void fillBackground(GFXcanvas16 &gfx, int w, int h) {
  (void)w;
  (void)h;
  gfx.fillScreen(themeFlatColor());
}

uint16_t randomBrightColor() {
  float h = float(random(0, 3600)) / 10.0f;
  float s = float(random(78, 101)) / 100.0f;
  float v = float(random(88, 101)) / 100.0f;
  return vividHsvColor565(h, s, v);
}

float randf(float minValue, float maxValue) {
  return minValue + (maxValue - minValue) * (float(random(0, 10000)) / 9999.0f);
}

uint16_t scaleColor565(uint16_t color, float scale) {
  int r = int(float((color >> 11) & 0x1F) * scale);
  int g = int(float((color >> 5) & 0x3F) * scale);
  int b = int(float(color & 0x1F) * scale);

  if (r < 0) r = 0;
  if (g < 0) g = 0;
  if (b < 0) b = 0;
  if (r > 31) r = 31;
  if (g > 63) g = 63;
  if (b > 31) b = 31;

  return uint16_t((r << 11) | (g << 5) | b);
}

void resetParticle(Particle &p) {
  p.angle = randf(0.0f, 2.0f * PI);
  p.radius = randf(8.0f, float(RADIUS));
  p.radialSpeed = randf(-0.35f, 0.35f);
  p.tangentialBias = randf(-0.02f, 0.02f);
  p.phase = randf(0.0f, 2.0f * PI);
  p.orbit = randf(2.0f, 12.0f);
  p.stretch = randf(0.65f, 1.35f);
  p.color = randomBrightColor();
  p.accentColor = randomBrightColor();
  p.size = uint8_t(random(1, PARTICLE_SIZE_MAX + 1));
  p.facetCount = uint8_t(random(FACET_MIN, FACET_MAX + 1));
}

void initParticles() {
  for (int i = 0; i < PARTICLE_COUNT; ++i) {
    resetParticle(particles[i]);
  }
}

void resetBackgroundShard(BackgroundShard &s) {
  s.x = randf(18.0f, float(RENDER_W - 18));
  s.y = randf(18.0f, float(RENDER_H - 18));
  s.vx = randf(-0.55f, 0.55f);
  s.vy = randf(-0.55f, 0.55f);
  if (fabsf(s.vx) < 0.12f) s.vx = (s.vx < 0.0f ? -0.18f : 0.18f);
  if (fabsf(s.vy) < 0.12f) s.vy = (s.vy < 0.0f ? -0.18f : 0.18f);
  // Background shards should occupy most of the source view (150px+ diameter).
  s.size = randf(75.0f, 96.0f);
  s.angle = randf(0.0f, 2.0f * PI);
  s.angleSpeed = randf(-0.02f, 0.02f);
  if (fabsf(s.angleSpeed) < 0.005f) s.angleSpeed = (s.angleSpeed < 0.0f ? -0.007f : 0.007f);
  s.color = randomBrightColor();
}

void initBackgroundShards() {
  for (int i = 0; i < BG_SHARD_COUNT; ++i) {
    resetBackgroundShard(bgShards[i]);
  }
}

void setupSpiPins() {
  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
}

void setupDisplay() {
  setupSpiPins();
  tft.begin();
  tft.setRotation(0);
  tft.fillScreen(themeFlatColor());
}

} // namespace kaleido
