# VCU-i.MX8Mplus

Bare-metal port of the Teensy 4.1 VCU project's real-time control logic, targeting the
**Cortex-M7 core of an NXP i.MX 8M Plus** (Variscite VAR-SOM-MX8M-PLUS SOM) on a **Variscite
Symphony carrier board, revision v1.4a**.

**Status: skeleton only, CAN bring-up + D10 heartbeat LED scope. Compiles and links cleanly,
never flashed to hardware.** Verified 2026-08-22 with Arm GNU Toolchain 14.2.Rel1
(`arm-none-eabi-gcc` 14.2.1) — `make` builds `build/vcu-imx8mp.elf` and `.bin` with zero errors
(only the expected nano.specs stub warnings for unimplemented syscalls like `_read`/`_write`,
which this firmware never calls, plus a harmless "LOAD segment with RWX permissions" linker
note). Text+data fits comfortably within the TCM budget (~23KB used of 127KB `m_text`, ~2.5KB of
128KB `m_data`). A clean build is still not the same as a working one: **nothing has been
flashed to or run on real hardware**, so treat the peripheral bring-up logic itself (clock tree,
pin mux, FLEXCAN2 config, I2C3/PCA9534 config) as unverified beyond "matches NXP/Variscite
source and documentation."

## Why bare metal, no RTOS

Matches the existing Teensy VCU's `loop()` architecture: a plain superloop, no Arduino/mbed
ecosystem, no RTOS.

## Sibling projects

- `../VCU` — the original Teensy 4.1 VCU project. Unrelated to this port beyond being the source
  of the control logic being ported.

## Hardware target

- SOM: Variscite VAR-SOM-MX8M-PLUS (NXP i.MX 8M Plus, SDK part name `MIMX8ML8`, specifically
  `MIMX8ML8DVNLZ`)
- Carrier: Variscite Symphony, revision v1.4a
- Real-time core: the i.MX 8M Plus's single Cortex-M7 (~800MHz) — unlike the i.MX 8M Mini, the
  Plus variant has one M7, not an M4.
- CAN: FLEXCAN2, not FLEXCAN1 — confirmed from Variscite's own board file (see below), not
  guessed from silicon defaults.

The broader system this board is being built for also includes (not yet implemented in this
repo — see the project's own memory file `project_imx8mp_hardware_plan.md` if picking this back
up in a fresh session without that context):

- an AIW-213HU-001 GNSS/cellular module on the Symphony's mini-PCIe connector, with a manually
  added USB wire to route GNSS output to the carrier (the mini-PCIe slot doesn't wire that up by
  default)
- a Mikroe Shuttle click-board carrier with an OPTO click (switched 12V wake input), an OPTO 3
  click (other 12V I/O), and possibly a CAN FD click if a second CAN bus is needed
- a Notecard (reused from the `VCU` Teensy project) for SMS
- a minimalist Linux userspace on the A53 application cores running RealDash for the
  driver-facing display

None of that is wired into this M7 firmware yet; this repo currently only implements FLEXCAN2
bring-up on the M7.

## Toolchain

NXP MCUXpresso SDK conventions/APIs (`fsl_*.c/h` drivers) — this is an NXP part, not ST, so
STM32Cube/HAL does not apply here. Target with `arm-none-eabi-gcc`.

## Sourcing and verified facts

All vendored/adapted source under `Drivers/`, `startup/`, and `linker/`, and the board files
under `Core/` (`board.c/h`, `pin_mux.c/h`, `clock_config.c/h`), come from Variscite's own
MCUXpresso SDK fork:

- Repository: `github.com/varigit/freertos-variscite`
- Branch: `mcuxpresso_sdk_2.15.x-var02`
- Board directory: `boards/som_mx8mp`
- License: **BSD-3-Clause** — safe to vendor directly (see `THIRD_PARTY_LICENSES.md`).

`fsl_i2c.c/h` (added for the D10 heartbeat LED) was pulled directly from that same repo/branch
at `devices/MIMX8ML8/drivers/fsl_i2c.{c,h}` — vendored in full/unmodified, same as the other
`fsl_*` driver files, not hand-written against the register map.

Facts confirmed directly from that repo (not inferred):

- Device: `MIMX8ML8` / `CPU_MIMX8ML8DVNLZ` (NXP's SDK part-number naming).
- CAN peripheral: **FLEXCAN2**, from Variscite's own
  `boards/som_mx8mp/driver_examples/flexcan/interrupt_transfer/pin_mux.c` generator comment.
- Pin mux (same file):
  - `AJ4` → `FLEXCAN2` `can_rx`, via `IOMUXC_UART3_TXD_CAN2_RX` (pad shared with UART3_TXD on the
    i.MX8M's IOMUX — normal, not a wiring conflict).
  - `AE6` → `FLEXCAN2` `can_tx`, via `IOMUXC_UART3_RXD_CAN2_TX`.
- Clock: `CLOCK_SetRootMux(kCLOCK_RootFlexCan2, kCLOCK_FlexCanRootmuxSysPll1)`, kernel clock
  divided down from SYSTEM PLL1 (already running at 800MHz — configured by ROM/U-Boot before the
  M7 is kicked; **do not re-init the system PLLs on the M7 side**, the reference example
  explicitly warns this risks hanging the SoC). This project computes the actual kernel clock
  frequency at runtime via `CLOCK_GetPllFreq`/`CLOCK_GetRootPreDivider`/`CLOCK_GetRootPostDivider`
  rather than hardcoding it, matching Variscite's own pattern.
- Linker script choice: `MIMX8ML8xxxxx_cm7_ram.ld` (TCM-resident code/data, with a 16MB DDR
  window at `0x80000000` reserved for CM7 non-cacheable use — see the comment in `board.c`'s
  `BOARD_InitMemory`). This is correct for this boot model: the M7 doesn't power on independently
  at reset — it boots via **remoteproc**, loaded and kicked by Linux/U-Boot on the A53 side, by
  which point DDR is already initialized. The `_flash` and `_ddr_ram` linker scripts in the
  vendored SDK don't apply to this boot model.
- D10 heartbeat LED / I2C#A: cross-referenced from two independent Variscite PDFs (both in the
  repo root, not vendored SDK source), not guessed:
  - `Symphony-Board-Schematics.pdf`, sheet "03. SOM" (word bounding-box coordinates extracted
    directly to resolve the multi-column pin table, since the flattened text layout was
    unreliable — see git history for the derivation): SOM header pin 88 = `I2C#A_SCL`, pin 87 =
    `I2C#A_SDA`.
  - `VAR-SOM-MX8M-PLUS_Datasheet.pdf`, Table 52 (I2C3 Signals, p.84): pin 88 = `I2C3_SCL` alt0
    (ball `AJ7`), pin 87 = `I2C3_SDA` alt0 (ball `AJ6`). So **I2C#A is I2C3**, on its native/alt0
    pins — user-confirmed independently mid-session.
  - `Symphony-Board_Datasheet.pdf` §5.5.2.3: D10 is driven by P0 of U5, a PCA9534PWR I2C GPIO
    expander at address `0x20`, which also drives SW1-4 and several board-housekeeping signals
    (ENET1 reset, SOM voltage select, USB3/SATA select) — **not** a plain SoC GPIO pin.

**Not yet confirmed** — worth checking before trusting fully:

- Whether `boards/som_mx8mp` in this SDK fork represents the SOM specifically on the **Symphony
  v1.4a** carrier, or a different/generic Variscite carrier — the SDK board name doesn't
  distinguish carrier revisions. Check against Variscite's published Symphony v1.4a schematic
  (variscite.com) before trusting the pin assignments against real hardware.
- The `AJ4`/`AE6` pin assignments above are from the SDK's pin_mux generator, not pin-traced with
  a meter against the actual carrier's CAN connector.
- **Whether the M7 actually has RDC access to I2C3 at all.** `BOARD_RdcInit()` only moves the M7
  *core* into RDC domain 1 — it does not reassign individual peripherals. FLEXCAN2 works because
  Variscite's own device tree (`imx8mp-var-common-m7.dtsi`) already excludes it from Linux and
  grants the M7 domain access; there is **no equivalent confirmation for I2C3**. The GPIO
  expander on this bus also drives board-housekeeping signals Linux almost certainly still
  manages, so sharing it uncoordinated risks bus contention. `main.c` guards against this with an
  `RDC_GetPeriphAccessPolicy()` check before ever touching I2C3 — if the M7's domain hasn't been
  granted read+write access, the LED code silently does nothing rather than assume the bus is
  free. Whether that check ever actually passes on real hardware is unverified.

## Directory structure

```
Core/            main.c (original), board.c/h, pin_mux.c/h, clock_config.c/h (adapted)
Drivers/
  CMSIS/Include/ Cortex-M7 CMSIS core headers (vendored, BSD-3)
  MIMX8ML8/      device headers, system_*.c/h, drivers/ (fsl_common, fsl_common_arm, fsl_clock,
                 fsl_flexcan, fsl_gpio, fsl_rdc, fsl_i2c — vendored, BSD-3)
startup/         startup_MIMX8ML8_cm7.S (vendored, BSD-3)
linker/          MIMX8ML8xxxxx_cm7_ram.ld (vendored, BSD-3)
Makefile         arm-none-eabi-gcc build, never run
```

`Core/main.c` brings up memory/MPU (`BOARD_InitMemory`), RDC domain isolation
(`BOARD_RdcInit`), pins, clocks, then configures FLEXCAN2 for classic CAN at 500kbit/s (matching
the Teensy VCU's bus speed) and polls one Rx mailbox in a superloop — deliberately minimal,
matching the "CAN bring-up only" scope of the initial skeleton (no CAN FD, no interrupt-handle
based transfers, no RTOS). It also brings up SysTick (1ms tick) and, if the RDC grants I2C3
access, the D10 heartbeat LED (a PCA9534 GPIO expander pin, toggled every 500ms) — both
non-blocking, so neither ever stalls the CAN Rx poll.

## Building

```
make
```

Requires `arm-none-eabi-gcc` and GNU `make` on `PATH` (or pass
`TOOLCHAIN_PREFIX=/path/to/arm-none-eabi-`). Compiler flags in the `Makefile` were transcribed
from Variscite's own reference example build (`armgcc/{flags,config}.cmake` in the same
repo/branch) and confirmed to work with a real build on 2026-08-22 (Arm GNU Toolchain 14.2.Rel1).
One header had to be vendored in beyond what the handoff plan called out:
`Drivers/CMSIS/Include/cachel1_armv7.h`, pulled in transitively by `core_cm7.h` and missing from
the initial vendoring pass.

## Flashing / deployment (not attempted, not fully researched)

The i.MX 8M Plus's M7 core has no onboard debugger and doesn't power on independently at reset —
it's typically loaded and started by Linux via **remoteproc** — U-Boot or the Linux remoteproc
framework loads the built `.elf`/`.bin` into RAM and kicks the core, rather than the M7 booting
standalone at power-on. The exact mechanism
(U-Boot `bootaux` command vs. Linux `/sys/class/remoteproc/remoteproc0/firmware` +
`state`) has not been worked out for the Symphony v1.4a carrier's boot configuration yet.
