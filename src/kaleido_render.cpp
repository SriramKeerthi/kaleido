#include "kaleido.h"

namespace kaleido {

void updateParticleState(Particle &p) {
  p.radius += p.radialSpeed;
  p.angle += rotationSpeed + p.tangentialBias;

  if (p.radius < 6.0f) {
    p.radius = 6.0f;
    p.radialSpeed = -p.radialSpeed;
  }

  if (p.radius > float(RADIUS)) {
    p.radius = float(RADIUS);
    p.radialSpeed = -p.radialSpeed;
  }

  p.tangentialBias += randf(-0.0008f, 0.0008f);
  if (p.tangentialBias > 0.03f) p.tangentialBias = 0.03f;
  if (p.tangentialBias < -0.03f) p.tangentialBias = -0.03f;
  p.phase += 0.012f + 0.25f * p.tangentialBias;
  if (p.phase > 2.0f * PI) p.phase -= 2.0f * PI;
}

void drawRegularPolygon(
  Adafruit_GFX &gfx,
  int cx,
  int cy,
  int radius,
  uint8_t sides,
  float rotation,
  uint16_t color
) {
  if (sides < 3 || radius < 1) return;
  const float step = (2.0f * PI) / float(sides);
  int px = cx + int(cosf(rotation) * radius);
  int py = cy + int(sinf(rotation) * radius);
  for (uint8_t i = 1; i <= sides; ++i) {
    float a = rotation + step * float(i);
    int x = cx + int(cosf(a) * radius);
    int y = cy + int(sinf(a) * radius);
    gfx.drawLine(px, py, x, y, color);
    px = x;
    py = y;
  }
}

void drawFilledRegularPolygon(
  Adafruit_GFX &gfx,
  int cx,
  int cy,
  int radius,
  uint8_t sides,
  float rotation,
  uint16_t fillColor,
  uint16_t edgeColor
) {
  if (sides < 3 || radius < 1) return;

  const float step = (2.0f * PI) / float(sides);
  int xFirst = cx + int(cosf(rotation) * radius);
  int yFirst = cy + int(sinf(rotation) * radius);
  int xPrev = xFirst;
  int yPrev = yFirst;

  for (uint8_t i = 1; i <= sides; ++i) {
    float a = rotation + step * float(i);
    int x = cx + int(cosf(a) * radius);
    int y = cy + int(sinf(a) * radius);
    gfx.fillTriangle(cx, cy, xPrev, yPrev, x, y, fillColor);
    xPrev = x;
    yPrev = y;
  }

  drawRegularPolygon(gfx, cx, cy, radius, sides, rotation, edgeColor);
}

void drawThickArc(
  Adafruit_GFX &gfx,
  int cx,
  int cy,
  int radius,
  int thickness,
  float startA,
  float endA,
  uint16_t color
) {
  if (radius < 1 || thickness < 1) return;
  const int segs = ARC_SEGMENTS;
  for (int t = 0; t < thickness; ++t) {
    float rr = float(radius + t);
    int px = cx + int(cosf(startA) * rr);
    int py = cy + int(sinf(startA) * rr);
    for (int s = 1; s <= segs; ++s) {
      float u = float(s) / float(segs);
      float a = startA + (endA - startA) * u;
      int x = cx + int(cosf(a) * rr);
      int y = cy + int(sinf(a) * rr);
      gfx.drawLine(px, py, x, y, color);
      px = x;
      py = y;
    }
  }
}

void drawBackgroundShards(Adafruit_GFX &gfx) {
  for (int i = 0; i < BG_SHARD_COUNT; ++i) {
    BackgroundShard &s = bgShards[i];
    s.x += s.vx;
    s.y += s.vy;
    s.angle += s.angleSpeed;
    if (s.angle > 2.0f * PI) s.angle -= 2.0f * PI;
    if (s.angle < 0.0f) s.angle += 2.0f * PI;

    float margin = s.size * 0.5f + 2.0f;
    if (s.x < margin) {
      s.x = margin;
      s.vx = fabsf(s.vx);
    } else if (s.x > float(RENDER_W) - margin) {
      s.x = float(RENDER_W) - margin;
      s.vx = -fabsf(s.vx);
    }
    if (s.y < margin) {
      s.y = margin;
      s.vy = fabsf(s.vy);
    } else if (s.y > float(RENDER_H) - margin) {
      s.y = float(RENDER_H) - margin;
      s.vy = -fabsf(s.vy);
    }

    int cx = int(s.x);
    int cy = int(s.y);
    uint16_t fill = scaleColor565(s.color, 0.28f);
    uint16_t edge = scaleColor565(s.color, 0.55f);
    drawFilledRegularPolygon(gfx, cx, cy, int(s.size), 3, s.angle, fill, edge);
  }
}

void drawModeFloral(Adafruit_GFX &gfx, const Particle &p, float a, uint16_t color) {
  float sizeScale = (0.75f + 0.18f * float(p.size)) * PARTICLE_VISUAL_SCALE;
  float innerR = 6.0f + p.radius * 0.56f;
  float outerR = p.radius + (sizeScale - 1.0f) * 5.0f;
  float spread = 0.03f + 0.016f * float(p.size);

  int xi = int(RENDER_CENTER_X + cosf(a) * innerR);
  int yi = int(RENDER_CENTER_Y + sinf(a) * innerR);
  int xoL = int(RENDER_CENTER_X + cosf(a - spread) * outerR);
  int yoL = int(RENDER_CENTER_Y + sinf(a - spread) * outerR);
  int xoR = int(RENDER_CENTER_X + cosf(a + spread) * outerR);
  int yoR = int(RENDER_CENTER_Y + sinf(a + spread) * outerR);

  uint16_t fill = scaleColor565(color, 0.70f);
  uint16_t edge = scaleColor565(color, 1.25f);
  uint16_t glow = scaleColor565(color, 1.45f);

  gfx.fillTriangle(xi, yi, xoL, yoL, xoR, yoR, fill);
  gfx.drawLine(xi, yi, xoL, yoL, edge);
  gfx.drawLine(xi, yi, xoR, yoR, edge);
  gfx.drawLine(xoL, yoL, xoR, yoR, glow);
  gfx.fillCircle((xoL + xoR) / 2, (yoL + yoR) / 2, 1 + (p.size / 2), edge);

  int mxL = (2 * xi + xoL) / 3;
  int myL = (2 * yi + yoL) / 3;
  int mxR = (2 * xi + xoR) / 3;
  int myR = (2 * yi + yoR) / 3;
  gfx.drawLine(mxL, myL, mxR, myR, glow);
  gfx.fillTriangle(mxL, myL, (xi + mxL) / 2, (yi + myL) / 2, (xi + mxR) / 2, (yi + myR) / 2, scaleColor565(color, 0.52f));

  for (uint8_t j = 0; j < p.facetCount; ++j) {
    float t = float(j + 1) / float(p.facetCount + 1);
    float aa = a + (t - 0.5f) * 0.18f + sinf(p.phase + t * 4.0f) * 0.02f;
    float rr = p.radius * (0.65f + 0.3f * t) + p.orbit * sinf(p.phase + t * 6.0f);
    int xc = int(RENDER_CENTER_X + cosf(aa) * rr);
    int yc = int(RENDER_CENTER_Y + sinf(aa) * rr);
    uint16_t c = (j & 1) ? scaleColor565(p.accentColor, 0.9f) : scaleColor565(color, 0.9f);
    if (ENABLE_POLYGONS) {
      uint16_t sqFill = scaleColor565((j & 1) ? p.accentColor : color, 0.85f);
      uint16_t sqEdge = scaleColor565((j & 1) ? p.accentColor : color, 1.10f);
      drawFilledRegularPolygon(
        gfx,
        xc,
        yc,
        int((1 + (p.size / 2) + (j % 2)) * FEATURE_SHAPE_SCALE),
        4,
        p.phase + t * 2.0f,
        sqFill,
        sqEdge
      );
    } else {
      gfx.fillCircle(xc, yc, int((1 + (p.size > 3 ? 1 : 0) + (j % 2)) * FEATURE_SHAPE_SCALE), c);
    }
  }
}

void drawModeShard(Adafruit_GFX &gfx, const Particle &p, float a, uint16_t color) {
  float sizeScale = (0.75f + 0.18f * float(p.size)) * PARTICLE_VISUAL_SCALE;
  float r0 = 8.0f + p.radius * 0.22f * sizeScale;
  float r1 = 10.0f + p.radius * 0.66f * sizeScale;
  float r2 = min(float(RADIUS), p.radius + 8.0f + 2.5f * float(p.size));
  float twist = 0.10f * sinf(a * 3.0f + p.radius * 0.06f + rotationAngle * 2.0f);

  int x0 = int(RENDER_CENTER_X + cosf(a + twist) * r0);
  int y0 = int(RENDER_CENTER_Y + sinf(a + twist) * r0);
  int x1 = int(RENDER_CENTER_X + cosf(a - 0.06f + twist) * r1);
  int y1 = int(RENDER_CENTER_Y + sinf(a - 0.06f + twist) * r1);
  int x2 = int(RENDER_CENTER_X + cosf(a + 0.06f + twist) * r2);
  int y2 = int(RENDER_CENTER_Y + sinf(a + 0.06f + twist) * r2);

  uint16_t fill = scaleColor565(color, 0.62f);
  uint16_t edge = scaleColor565(color, 1.18f);
  uint16_t hot = scaleColor565(color, 1.42f);

  gfx.fillTriangle(x0, y0, x1, y1, x2, y2, fill);
  gfx.drawLine(x0, y0, x1, y1, edge);
  gfx.drawLine(x0, y0, x2, y2, edge);
  gfx.drawLine(x1, y1, x2, y2, edge);

  int xm = (x1 + x2) / 2;
  int ym = (y1 + y2) / 2;
  int xi = (x0 + xm) / 2;
  int yi = (y0 + ym) / 2;
  gfx.drawLine(x0, y0, xm, ym, hot);
  gfx.drawLine(x1, y1, xi, yi, hot);
  gfx.drawLine(x2, y2, xi, yi, hot);
  gfx.fillTriangle(xi, yi, xm, ym, x2, y2, scaleColor565(color, 0.48f));

  for (uint8_t j = 0; j < p.facetCount; ++j) {
    float off = (float(j) - float(p.facetCount) * 0.5f) * 0.05f;
    float aa = a + off + 0.04f * sinf(p.phase + j * 1.7f);
    float rA = p.radius * (0.45f + 0.12f * j);
    float rB = rA + 8.0f + 6.0f * sinf(p.phase + j * 2.4f);
    int xa = int(RENDER_CENTER_X + cosf(aa) * rA);
    int ya = int(RENDER_CENTER_Y + sinf(aa) * rA);
    int xb = int(RENDER_CENTER_X + cosf(aa + 0.03f) * rB);
    int yb = int(RENDER_CENTER_Y + sinf(aa + 0.03f) * rB);
    gfx.drawLine(xa, ya, xb, yb, scaleColor565(p.accentColor, 0.85f));
    gfx.drawPixel(xb, yb, hot);

    int pcx = (xa + xb) / 2;
    int pcy = (ya + yb) / 2;
    if (ENABLE_POLYGONS) {
      uint16_t pentBase = scaleColor565(p.accentColor, 0.95f);
      drawFilledRegularPolygon(
        gfx,
        pcx,
        pcy,
        int((1 + (p.size / 2) + (j % 2)) * FEATURE_SHAPE_SCALE),
        5,
        p.phase + off * 7.0f,
        scaleColor565(pentBase, 0.82f),
        scaleColor565(pentBase, 1.18f)
      );
    }
  }
}

void drawModeSpark(Adafruit_GFX &gfx, const Particle &p, float a, uint16_t color) {
  float sizeScale = (0.75f + 0.18f * float(p.size)) * PARTICLE_VISUAL_SCALE;
  int x = int(RENDER_CENTER_X + cosf(a) * p.radius);
  int y = int(RENDER_CENTER_Y + sinf(a) * p.radius);

  float echoR = 6.0f + p.radius * 0.78f * sizeScale;
  int xe = int(RENDER_CENTER_X + cosf(a + 0.02f) * echoR);
  int ye = int(RENDER_CENTER_Y + sinf(a + 0.02f) * echoR);

  uint16_t bright = scaleColor565(color, 1.28f);
  uint16_t dim = scaleColor565(color, 0.60f);
  uint16_t core = scaleColor565(color, 1.50f);
  uint16_t tint = scaleColor565(color, 0.45f);

  gfx.drawPixel(x, y, bright);
  gfx.drawPixel(x + 1, y, dim);
  gfx.drawPixel(x - 1, y, dim);
  gfx.drawPixel(x, y + 1, dim);
  gfx.drawPixel(x, y - 1, dim);
  gfx.drawLine(x, y, xe, ye, dim);
  gfx.fillCircle(xe, ye, 1, bright);

  int x2 = int(RENDER_CENTER_X + cosf(a - 0.03f) * (p.radius * 0.92f));
  int y2 = int(RENDER_CENTER_Y + sinf(a - 0.03f) * (p.radius * 0.92f));
  int x3 = int(RENDER_CENTER_X + cosf(a + 0.05f) * (p.radius * 0.70f + 10.0f));
  int y3 = int(RENDER_CENTER_Y + sinf(a + 0.05f) * (p.radius * 0.70f + 10.0f));
  gfx.drawLine(x, y, x2, y2, tint);
  gfx.drawLine(x, y, x3, y3, tint);
  gfx.fillTriangle(x, y, x2, y2, x3, y3, scaleColor565(color, 0.35f));
  gfx.fillCircle((x + x2) / 2, (y + y2) / 2, 1, core);

  int prevX = 0;
  int prevY = 0;
  bool hasPrev = false;
  for (uint8_t j = 0; j < p.facetCount + 1; ++j) {
    float t = float(j) / float(p.facetCount + 1);
    float aa = a + 0.06f * sinf(p.phase + t * 8.0f);
    float rr = p.radius * (0.35f + 0.55f * t) * p.stretch;
    int xb = int(RENDER_CENTER_X + cosf(aa) * rr);
    int yb = int(RENDER_CENTER_Y + sinf(aa) * rr);
    uint16_t bead = (j & 1) ? scaleColor565(color, 1.1f) : scaleColor565(p.accentColor, 0.9f);
    gfx.fillCircle(xb, yb, int((1 + (p.size >= 5 ? 1 : 0)) * FEATURE_SHAPE_SCALE), bead);
    if (hasPrev) {
      gfx.drawLine(prevX, prevY, xb, yb, scaleColor565(bead, 0.6f));
    }
    prevX = xb;
    prevY = yb;
    hasPrev = true;
  }

  float arcSpan = 0.18f + 0.05f * float(p.size);
  if (ENABLE_THICK_ARCS) {
    drawThickArc(
      gfx,
      RENDER_CENTER_X,
      RENDER_CENTER_Y,
      int(p.radius * p.stretch * (0.85f + 0.1f * float(p.size))),
      int((1 + (p.size / 2)) * FEATURE_SHAPE_SCALE),
      a - arcSpan,
      a + arcSpan,
      scaleColor565(p.accentColor, 0.75f)
    );
  }
}

void drawParticleAt(Adafruit_GFX &gfx, const Particle &p, float a, uint16_t color) {
  switch (drawMode) {
    case 0:
      drawModeFloral(gfx, p, a, color);
      break;
    case 1:
      drawModeShard(gfx, p, a, color);
      break;
    case 2:
      drawModeSpark(gfx, p, a, color);
      break;
  }
}

void drawSourceCore(Adafruit_GFX &gfx) {
  uint16_t inner = (themeMode == 0) ? makeColor565(255, 210, 140) : makeColor565(90, 70, 40);
  uint16_t mid = (themeMode == 0) ? makeColor565(180, 95, 40) : makeColor565(120, 85, 45);
  uint16_t outer = (themeMode == 0) ? makeColor565(80, 35, 20) : makeColor565(150, 110, 70);

  gfx.fillCircle(RENDER_CENTER_X, RENDER_CENTER_Y, 2, inner);
  gfx.drawCircle(RENDER_CENTER_X, RENDER_CENTER_Y, 4, mid);
  gfx.drawCircle(RENDER_CENTER_X, RENDER_CENTER_Y, 7, outer);
}

void applyTriangleMask(Adafruit_GFX &gfx) {
  (void)gfx;
  uint16_t *buf = frameCanvas->getBuffer();
  if (buf == nullptr) return;

  uint16_t flat = backgroundColor();
  static constexpr int TX = TRI_TOP_X - RENDER_X0;
  static constexpr int TY = TRI_TOP_Y - RENDER_Y0 - TRI_OVERSCAN;
  static constexpr int LX = TRI_BL_X - RENDER_X0 - TRI_OVERSCAN;
  static constexpr int LY = TRI_BL_Y - RENDER_Y0 + TRI_OVERSCAN;
  static constexpr int RX = TRI_BR_X - RENDER_X0 + TRI_OVERSCAN;

  for (int y = 0; y < RENDER_H; ++y) {
    float t = float(y - TY) / float(LY - TY);
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    int xLeft = int(float(TX) + (float(LX - TX) * t));
    int xRight = int(float(TX) + (float(RX - TX) * t));
    if (xLeft > xRight) {
      int tmp = xLeft;
      xLeft = xRight;
      xRight = tmp;
    }
    if (xLeft < 0) xLeft = 0;
    if (xRight > (RENDER_W - 1)) xRight = (RENDER_W - 1);

    int row = y * RENDER_W;
    if (xLeft > 0) {
      for (int x = 0; x < xLeft; ++x) {
        buf[row + x] = flat;
      }
    }
    if (xRight < (RENDER_W - 1)) {
      for (int x = xRight + 1; x < RENDER_W; ++x) {
        buf[row + x] = flat;
      }
    }
  }
}

void renderNextFrameOffscreen() {
  fillBackground(*frameCanvas, RENDER_W, RENDER_H);
  drawBackgroundShards(*frameCanvas);

  rotationAngle += rotationSpeed;
  if (rotationAngle > 2.0f * PI) {
    rotationAngle -= 2.0f * PI;
  }

  for (int i = 0; i < PARTICLE_COUNT; ++i) {
    Particle &p = particles[i];
    updateParticleState(p);
    float baseA = p.angle + rotationAngle;
    drawParticleAt(*frameCanvas, p, baseA, p.color);

    if (RENDER_PASSES >= 2) {
      float sideA = baseA + 0.04f + p.tangentialBias * 3.0f;
      uint16_t sideColor = scaleColor565(p.color, 0.70f);
      drawParticleAt(*frameCanvas, p, sideA, sideColor);
    }

    if (RENDER_PASSES >= 3) {
      float edgeA = baseA - 0.06f - p.tangentialBias * 2.0f;
      uint16_t edgeColor = scaleColor565(p.color, 0.45f);
      drawParticleAt(*frameCanvas, p, edgeA, edgeColor);
    }
  }

  drawSourceCore(*frameCanvas);
  applyTriangleMask(*frameCanvas);
  tft.drawRGBBitmap(RENDER_X0, RENDER_Y0, frameCanvas->getBuffer(), RENDER_W, RENDER_H);
}

void printStatus() {
  static uint32_t lastPrint = 0;
  uint32_t now = millis();
  if (now - lastPrint >= 1000) {
    Serial.print("speed=");
    Serial.print(rotationSpeed, 4);
    Serial.print(" mode=");
    Serial.print(drawMode);
    if (themeMode == 0) Serial.print("D");
    else if (themeMode == 1) Serial.print("L");
    else Serial.print("Y");
    Serial.print(" btn=");
    Serial.println(buttonStablePressed ? "down" : "up");
    lastPrint = now;
  }
}

} // namespace kaleido
