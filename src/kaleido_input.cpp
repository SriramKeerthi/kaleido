#include "kaleido.h"

namespace kaleido {

void advanceMode() {
  uint8_t cycleMode = drawMode + (themeMode * 3);
  cycleMode = (cycleMode + 1) % 9;
  drawMode = cycleMode % 3;
  themeMode = cycleMode / 3;
}

void cycleSpeedPreset() {
  static constexpr float SPEED_PRESETS[] = {0.0060f, 0.0125f, 0.0220f, 0.0340f};
  speedPresetIndex = (speedPresetIndex + 1) % (sizeof(SPEED_PRESETS) / sizeof(SPEED_PRESETS[0]));
  rotationSpeed = SPEED_PRESETS[speedPresetIndex];
}

void randomizePattern() {
  initParticles();
  initBackgroundShards();
  rotationAngle = randf(0.0f, 2.0f * PI);
}

void updateButtonControls() {
  uint32_t now = millis();
  bool rawPressed = (digitalRead(UI_BUTTON_PIN) == LOW);

  if (rawPressed != buttonLastRawPressed) {
    buttonLastRawPressed = rawPressed;
    buttonLastEdgeMs = now;
  }

  if (now - buttonLastEdgeMs < BUTTON_DEBOUNCE_MS) {
    return;
  }

  if (rawPressed != buttonStablePressed) {
    buttonStablePressed = rawPressed;
    if (buttonStablePressed) {
      buttonPressedMs = now;
      buttonLongHandled = false;
    } else {
      buttonReleasedMs = now;
      if (!buttonLongHandled) {
        ++pendingClicks;
      }
    }
  }

  if (buttonStablePressed && !buttonLongHandled && (now - buttonPressedMs >= BUTTON_LONG_MS)) {
    randomizePattern();
    buttonLongHandled = true;
    pendingClicks = 0;
    return;
  }

  if (!buttonStablePressed && pendingClicks > 0 && (now - buttonReleasedMs >= BUTTON_DOUBLE_MS)) {
    if (pendingClicks == 1) {
      advanceMode();
    } else {
      cycleSpeedPreset();
    }
    pendingClicks = 0;
  }
}

} // namespace kaleido
