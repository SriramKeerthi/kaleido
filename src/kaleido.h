#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>

namespace kaleido {

#if defined(CONFIG_IDF_TARGET_ESP32C3)
constexpr int TFT_SCLK = 4;
constexpr int TFT_MOSI = 6;
constexpr int TFT_CS = 7;
constexpr int TFT_DC = 2;
constexpr int TFT_RST = 3;
constexpr int UI_BUTTON_PIN = 9;
#else
constexpr int TFT_SCLK = 18;
constexpr int TFT_MOSI = 23;
constexpr int TFT_CS = 16;
constexpr int TFT_DC = 4;
constexpr int TFT_RST = 17;
constexpr int UI_BUTTON_PIN = 0;
#endif

constexpr int SCREEN_W = 240;
constexpr int SCREEN_H = 240;
constexpr int CENTER_X = SCREEN_W / 2;
constexpr int CENTER_Y = SCREEN_H / 2;
constexpr int RADIUS = 118;

constexpr float COS30 = 0.8660254f;
constexpr int TRI_TOP_X = CENTER_X;
constexpr int TRI_TOP_Y = CENTER_Y - RADIUS;
constexpr int TRI_BR_X = CENTER_X + int(RADIUS * COS30);
constexpr int TRI_BR_Y = CENTER_Y + int(RADIUS * 0.5f);
constexpr int TRI_BL_X = CENTER_X - int(RADIUS * COS30);
constexpr int TRI_BL_Y = TRI_BR_Y;

// Draw a little beyond the ideal mirror triangle to tolerate alignment error.
constexpr int TRI_OVERSCAN = 10;
constexpr int RENDER_X0 = ((TRI_BL_X - TRI_OVERSCAN) < 0) ? 0 : (TRI_BL_X - TRI_OVERSCAN);
constexpr int RENDER_Y0 = ((TRI_TOP_Y - TRI_OVERSCAN) < 0) ? 0 : (TRI_TOP_Y - TRI_OVERSCAN);
constexpr int RENDER_X1 = ((TRI_BR_X + TRI_OVERSCAN) > (SCREEN_W - 1)) ? (SCREEN_W - 1) : (TRI_BR_X + TRI_OVERSCAN);
constexpr int RENDER_Y1 = ((TRI_BR_Y + TRI_OVERSCAN) > (SCREEN_H - 1)) ? (SCREEN_H - 1) : (TRI_BR_Y + TRI_OVERSCAN);
constexpr int RENDER_W = (RENDER_X1 - RENDER_X0 + 1);
constexpr int RENDER_H = (RENDER_Y1 - RENDER_Y0 + 1);
constexpr int RENDER_CENTER_X = CENTER_X - RENDER_X0;
constexpr int RENDER_CENTER_Y = CENTER_Y - RENDER_Y0;

#if defined(CONFIG_IDF_TARGET_ESP32C3)
constexpr int PARTICLE_COUNT = 20;
constexpr uint8_t FACET_MIN = 1;
constexpr uint8_t FACET_MAX = 2;
constexpr uint8_t PARTICLE_SIZE_MAX = 7;
constexpr uint8_t RENDER_PASSES = 1;
constexpr bool ENABLE_POLYGONS = true;
constexpr bool ENABLE_THICK_ARCS = true;
constexpr int ARC_SEGMENTS = 4;
constexpr uint32_t FRAME_MS = 24;
constexpr float PARTICLE_VISUAL_SCALE = 1.45f;
constexpr int BG_SHARD_COUNT = 3;
#else
constexpr int PARTICLE_COUNT = 52;
constexpr uint8_t FACET_MIN = 2;
constexpr uint8_t FACET_MAX = 5;
constexpr uint8_t PARTICLE_SIZE_MAX = 6;
constexpr uint8_t RENDER_PASSES = 3;
constexpr bool ENABLE_POLYGONS = true;
constexpr bool ENABLE_THICK_ARCS = true;
constexpr int ARC_SEGMENTS = 10;
constexpr uint32_t FRAME_MS = 10;
constexpr float PARTICLE_VISUAL_SCALE = 1.00f;
constexpr int BG_SHARD_COUNT = 6;
#endif

constexpr float MIN_SPEED = 0.0025f;
constexpr float MAX_SPEED = 0.0800f;
constexpr float SPEED_STEP = 0.0025f;
constexpr float FEATURE_SHAPE_SCALE = 3.0f;

constexpr uint32_t BUTTON_DEBOUNCE_MS = 25;
constexpr uint32_t BUTTON_DOUBLE_MS = 280;
constexpr uint32_t BUTTON_LONG_MS = 700;

struct Particle {
  float angle;
  float radius;
  float radialSpeed;
  float tangentialBias;
  float phase;
  float orbit;
  float stretch;
  uint16_t color;
  uint16_t accentColor;
  uint8_t size;
  uint8_t facetCount;
};

struct BackgroundShard {
  float x;
  float y;
  float vx;
  float vy;
  float size;
  float angle;
  float angleSpeed;
  uint16_t color;
};

extern Adafruit_GC9A01A tft;
extern GFXcanvas16 *frameCanvas;
extern Particle particles[PARTICLE_COUNT];
extern BackgroundShard bgShards[BG_SHARD_COUNT];

extern float rotationAngle;
extern float rotationSpeed;
extern uint8_t drawMode;
extern uint8_t themeMode;
extern uint8_t lastModeCycle;

extern bool buttonLastRawPressed;
extern bool buttonStablePressed;
extern bool buttonLongHandled;
extern uint8_t pendingClicks;
extern uint8_t speedPresetIndex;
extern uint32_t buttonLastEdgeMs;
extern uint32_t buttonPressedMs;
extern uint32_t buttonReleasedMs;
extern uint32_t lastFrameMs;

uint16_t makeColor565(uint8_t r, uint8_t g, uint8_t b);
uint16_t themeFlatColor();
uint16_t backgroundColor();
void fillBackground(GFXcanvas16 &gfx, int w, int h);
uint16_t randomBrightColor();
float randf(float minValue, float maxValue);
uint16_t scaleColor565(uint16_t color, float scale);

void resetParticle(Particle &p);
void initParticles();
void resetBackgroundShard(BackgroundShard &s);
void initBackgroundShards();

void setupSpiPins();
void setupDisplay();

void advanceMode();
void cycleSpeedPreset();
void randomizePattern();
void updateButtonControls();

void updateParticleState(Particle &p);
void drawRegularPolygon(Adafruit_GFX &gfx, int cx, int cy, int radius, uint8_t sides, float rotation, uint16_t color);
void drawThickArc(Adafruit_GFX &gfx, int cx, int cy, int radius, int thickness, float startA, float endA, uint16_t color);
void drawFilledRegularPolygon(
  Adafruit_GFX &gfx,
  int cx,
  int cy,
  int radius,
  uint8_t sides,
  float rotation,
  uint16_t fillColor,
  uint16_t edgeColor
);

void drawBackgroundShards(Adafruit_GFX &gfx);
void drawModeFloral(Adafruit_GFX &gfx, const Particle &p, float a, uint16_t color);
void drawModeShard(Adafruit_GFX &gfx, const Particle &p, float a, uint16_t color);
void drawModeSpark(Adafruit_GFX &gfx, const Particle &p, float a, uint16_t color);
void drawParticleAt(Adafruit_GFX &gfx, const Particle &p, float a, uint16_t color);
void drawSourceCore(Adafruit_GFX &gfx);
void applyTriangleMask(Adafruit_GFX &gfx);
void renderNextFrameOffscreen();

void printStatus();

} // namespace kaleido
