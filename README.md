# kaleido
Digital Kaleidoscope

## PlatformIO environments

- `esp32dev` -> ESP32 Dev Module
- `esp32c3` -> ESP32-C3 DevKitM-1 / C3 Mini-class target

CLI examples:

```bash
pio run -e esp32dev -t upload
pio run -e esp32c3 -t upload
pio device monitor -b 115200
```

## Display wiring

### ESP32 (`env:esp32dev`)

```text
3V3     -> VCC
GND     -> GND
GPIO18  -> SCL
GPIO23  -> SDA (MOSI)
GPIO4   -> DC
GPIO16  -> CS
GPIO17  -> RST
```

### ESP32-C3 (`env:esp32c3`)

Use these C3 pin mappings:

```text
3V3     -> VCC -> RED
GND     -> GND -> BLACK
GPIO4   -> SCL -> ORANGE
GPIO6   -> SDA (MOSI) -> GREEN
GPIO2   -> DC -> YELLOW
GPIO7   -> CS -> BLUE
GPIO3   -> RST -> BROWN
```

Note: if your C3 board routes SPI differently, change the constants in `src/main.cpp`.

## Single-button controls

No touch controls are used.

- Single click: next mode (`0D -> 1D -> 2D -> 0L -> 1L -> 2L -> ...`)
- Double click: cycle speed preset
- Long press: randomize pattern

Button pin used by code:

- ESP32: `GPIO0` (BOOT)
- ESP32-C3: `GPIO9` (BOOT on many boards)

## PlatformIO IDE: choose the correct board for Build/Upload buttons

In VS Code with PlatformIO extension:

1. Open PlatformIO sidebar -> `PROJECT TASKS`.
2. Expand the environment you want (`esp32dev` or `esp32c3`).
3. Click `Build` / `Upload` under that environment.

This is the safest method because it always targets the selected environment.

Optional default behavior:

- Set `[platformio] default_envs = esp32dev` or `esp32c3` in `platformio.ini`.
- The top-level toolbar Build/Upload buttons will then use that default env.
