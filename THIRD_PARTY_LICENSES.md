# Third-party licenses

This project vendors source files from NXP's MCUXpresso SDK for the i.MX 8M Plus, taken from
Variscite's own SDK fork so that the `boards/som_mx8mp` board-support files are included:

- Repository: `https://github.com/varigit/freertos-variscite`
- Branch: `mcuxpresso_sdk_2.15.x-var02`
- License: **BSD-3-Clause** (see full text below; the repo's own `COPYING-BSD-3` file matches)

Everything under `Drivers/`, and `startup/startup_MIMX8ML8_cm7.S`, and
`linker/MIMX8ML8xxxxx_cm7_ram.ld` originates from that repository, either vendored unmodified or
adapted (adaptations are noted with a comment at the top of the file explaining what was
trimmed/changed and why). `Core/board.c`, `Core/board.h`, `Core/pin_mux.c`, `Core/pin_mux.h`,
`Core/clock_config.c`, and `Core/clock_config.h` are adapted from
`boards/som_mx8mp/driver_examples/flexcan/interrupt_transfer/` in the same repo/branch.
`Core/main.c` is an original file written for this project (not derived from any NXP/Variscite
example file line-for-line), but it calls into and follows the initialization sequence
established by the vendored/adapted files above.

Per-file copyright holders (see each file's own header for the precise years/entity):
Freescale Semiconductor, Inc. and NXP (device headers, drivers, startup, linker scripts, CMSIS
Core), and NXP (board/pin_mux/clock_config example files this project adapted from).

No GPL-licensed code was used or referenced for this project — Variscite's SDK fork is
BSD-3-Clause throughout.

## BSD 3-Clause License text

```
The BSD 3 Clause License

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
may be used to endorse or promote products derived from this software without
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```
