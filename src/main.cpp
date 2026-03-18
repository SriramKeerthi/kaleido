#include "kaleido.h"

using namespace kaleido;

void setup() {
  Serial.begin(115200);
#if defined(CONFIG_IDF_TARGET_ESP32C3)
  uint32_t serialWaitStart = millis();
  while (!Serial && (millis() - serialWaitStart < 1500)) {
    delay(10);
  }
#endif
  delay(200);

  randomSeed(uint32_t(esp_random()));
  pinMode(UI_BUTTON_PIN, INPUT_PULLUP);

  setupDisplay();
  frameCanvas = new GFXcanvas16(RENDER_W, RENDER_H);
  if (frameCanvas != nullptr && frameCanvas->getBuffer() != nullptr) {
    Serial.println("Offscreen buffer enabled");
  } else {
    Serial.println("FATAL: Offscreen buffer allocation failed");
    while (true) {
      delay(1000);
    }
  }

  initParticles();
  initBackgroundShards();
  Serial.println("GC9A01 kaleidoscope start");
}

void loop() {
  uint32_t now = millis();

  updateButtonControls();

  if (now - lastFrameMs >= FRAME_MS) {
    lastFrameMs = now;
    uint8_t cycleMode = drawMode + (themeMode * 3);
    if (cycleMode != lastModeCycle) {
      tft.fillScreen(themeFlatColor());
      lastModeCycle = cycleMode;
    }
    renderNextFrameOffscreen();
  }

  printStatus();
}
