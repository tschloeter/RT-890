# Radtel RT-890 Enhancements to RT-890 Firmware

This repository contains additional modifications and feature enhancements built on top of the OEFW community firmware for the Radtel RT-890.

It is intended to support a slightly different use-case focus (radio scanning and RX-centric operation) while remaining fully compatible with the upstream firmware and its licensing model.

---

## Upstream Project

This repository is based on the work of the OEFW community and ultimately on DualTachyon’s open firmware reimplementation.

All credit for the original reverse engineering, firmware architecture, and the vast majority of functionality belongs to the upstream authors.

This repository **does not replace upstream** — it extends it.

If any of the changes here are useful for the broader community, contributions or merges back upstream are highly encouraged.

---

## Disclaimer / License

This repository is provided as an enhancement layer on top of the upstream OEFW firmware.

All original licensing terms are preserved and respected.

This firmware is still work in progress and may be unstable.  
Use entirely at your own risk.

Always back up your SPI memory before flashing any custom firmware.

---

## Changes Compared to Upstream (OEFWCOM main)

### Functional additions
- **FM Radio UI mode**
  - Fullscreen or Status Bar mode selectable at compile time  
  - `FM_RADIO_FULLSCREEN=0|1`  
  - Originally introduced in M70CM’s work

- **Radio Scanner Mode (RX-only firmware build)**
  - Compile-time option to completely disable transmission code  
  - `DISALLOW_TRANSMIT=1`

### Menu improvements
- Improved readability
- More logical entry order
- Menu entries conditionally compiled based on enabled features

### Code quality improvements (no functional change)
- Indentation cleanup
- Fixed incorrect comparison of `gSettings.BandInfo[]` against `BAND` enum in `radio.c:TuneTX()`
- Replaced magic numbers for side key indices with macros
- Replaced raw SPI flash base addresses with descriptive macros
- Replaced inline band frequency limits with symbolic macros

### Fixes
- Implemented previously unhandled `gVfoState[gSettings.CurrentVfo].bIs24Bit` logic in `menu.c`

---

## Repository Purpose

This fork exists because the modifications here target a slightly different operational profile (scanner-focused / TX-restricted builds) and therefore may evolve at a different pace than upstream.

Where possible, compatibility with upstream is maintained to simplify rebasing and merging.

---

## Keeping This Fork Updated

To sync with upstream development:

```
git fetch upstream
git rebase upstream/main
```

Resolve conflicts if necessary, then push updates.

---

## Attribution

This project builds upon:

- DualTachyon’s reverse engineering and firmware reimplementation
- The OEFW community firmware development
- Contributions from various community forks and experiments

Their work made this project possible.

---

# Upstream README (Unmodified)

The following documentation is preserved from the upstream project for completeness and reference.

# Radtel RT-890 Custom Firmware

This project is an effort to improve the firmware of the Radtel RT-890 in terms of features and radio performance.

It is based on [DualTachyon's OEFW](https://github.com/OEFW-community/radtel-rt-890-oefw) which is reversed from the original Radtel 1.34 firmware.  
Thanks to him for making this possible!

## Disclaimer
This firmware is a work in progress and could be unstable; it could alter your radio and its data.  
Use at your own risk and remember to [back up your SPI memory](https://github.com/OEFW-community/RT-890-custom-firmware/wiki/SPI) before installing any custom firmware.  DO NOT SKIP THIS STEP.

## Features
- All stock features: [check user's manual](https://cdn.shopifycdn.net/s/files/1/0564/8855/8800/files/RT-890_user_manual.pdf?v=1670288968)
- RX frequency can be set from 10 to 1300 MHz (results may vary)
- SSB reception
- Light theme
- AM Fix (improvement in AM reception with strong signals, port of @OneOfEleven's excellent work on the Quansheng UV-K5)
- Sensitivty improvements in gain and squelch 
- Full control over side key and main key shortcuts
- New configurable actions (FM Radio, Scanner, FLashlight)
- 0.01K step
- Displaying registers in single VFO mode
- Displaying dBM when receiving
- Reworked scanner
  - 8 Scan lists
  - Faster scanning
  - Resume mode: Time, Carrier, No
  - Change scan direction while scanning (up/down keys)
  - Force scan resume (up/down keys)
- Spectrum view with waterfall display
- Reworked main menu
- Ability to disable LED toggling when scanning
- And much more!

## Usage and feature instructions
See the [Wiki in this repository](https://github.com/OEFW-community/RT-890-custom-firmware/wiki) for detailed usage instructions.

## Pre-built firmware
You can find pre-built firmwares in the [Actions](https://github.com/OEFW-community/RT-890-custom-firmware/actions)

## Telegram group
If you want to discuss this project, you can join the [Telegram group](https://t.me/RT890_OEFW).


---
_Original OEFW readme_

# Support

* If you like my work, you can support me through https://ko-fi.com/DualTachyon

# Open reimplementation of the Radtel RT-890 v1.34 firmware

This repository is a preservation project of the Radtel RT-890 v1.34 firmware.
It is dedicated to understanding how the radio works and help developers making their own customisations/fixes/etc.
It is by no means fully understood or has all variables/functions properly named, as this is best effort only.
As a result, this repository will not include any customisations or improvements over the original firmware.

# Compiler

arm-none-eabi GCC version 10.3.1 is recommended, which is the current version on Ubuntu 22.04.03 LTS.
Other versions may generate a flash file that is too big.
You can get an appropriate version from: https://developer.arm.com/downloads/-/gnu-rm

# Building

To build the firmware, you need to fetch the submodules and then run make:
```
git submodule update --init --recursive --depth=1
make
```

# Flashing

* Use the firmware.bin file with either [RT-890-Flasher](https://github.com/OEFW-community/radtel-rt-890-flasher) or [RT-890-Flasher-CLI](https://github.com/OEFW-community/radtel-rt-890-flasher-cli)

# License

Copyright 2023 Dual Tachyon
https://github.com/DualTachyon

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

