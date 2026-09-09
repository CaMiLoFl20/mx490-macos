# MX490 firmware evidence and recovery paths

## Finding

The missing artifact is not another protocol implementation. It is a ground-truth
runtime/flash image or a reliable way to observe the HTTP registration data while
the printer is running. Static analysis of the decrypted 4.040 update has not
identified a trustworthy dispatch-table address.

The strongest route found in comparable Canon PIXMA research is a **read-only
SPI flash dump**. Recent PIXMA work describes dumping the board flash with a
Raspberry Pi SPI controller and `flashrom`, then comparing multiple reads before
any modification.[1] This bypasses the Canon updater and gives the complete
bootloader, configuration, compressed runtime, and integrity metadata in one
artifact. It requires physical access to the board and an electrical-level
identification of the flash chip; it should be treated as acquisition only.

## Ranked options

| Rank | Path | What it can provide | Cost/risk | Recommendation |
|---|---|---|---|---|
| 1 | Read the board's SPI flash in-circuit | Complete bootloader and runtime image; possible RAM/flash layout clues | Requires hardware access; incorrect voltage or powering both sides can damage the board | Best evidence path if the printer can be opened safely |
| 2 | Capture the Canon updater in download mode | Update framing, validation fields, addresses, and any diagnostic responses | No write should be attempted; download mode behavior is model-specific | Do this before hardware work if a spare Windows environment is available |
| 3 | Static/differential analysis of related PIXMA images | Function signatures, HTTP tables, decompression and boot patterns | Related models may use different SoCs or layouts | Useful for locating patterns, insufficient alone for a flashable MX490 image |
| 4 | Service-mode USB probing | Possible recovery/diagnostic endpoints | Public service documentation is for other Canon families; MX490 behavior is unverified | Probe only read-only USB descriptors and traffic |

## Why the updater path is promising but not enough

The 2014 Canon PIXMA research presented at HITB reports that PIXMA firmware
updates use encrypted/compressed S-record images and describes redirecting the
update mechanism through proxy settings.[2] Synacktiv's public tooling contains
Canon firmware decryption and decompression utilities.[3] Those references
support our existing 4.040 extraction work, but they do not prove that MX490
accepts an unsigned modified image or reveal its runtime HTTP callback table.

Canon's current MX490 download page identifies the available updater as version
4.040.[4] Canon's manual documents normal firmware installation and version
checking, but does not document a developer console, RAM dump, or debug mode.[5]

## Safe acquisition plan

1. **Do not send an update or write request.** Save the existing 4.040 updater,
   decrypted S-record, and decoded image hashes already recorded in this repo.
2. If using download mode, capture only USB/TCP traffic while the official tool
   performs device discovery and reads identity/version information. Stop before
   the update transfer or confirmation step.
3. If opening the printer, identify the flash part and logic voltage first. Read
   the chip with the printer unpowered, take at least two complete dumps, and
   require byte-for-byte agreement before analysis. Preserve the original dumps
   read-only and hash them immediately.
4. Compare the physical dump with the decoded 4.040 image. The comparison will
   tell us whether the update image omits boot/configuration regions and may
   reveal the exact runtime relocation and integrity scheme.
5. Only after those checks should we search the physical image for live HTTP
   route tables, callback pointers, and unused executable space.

## What would unblock the native build

Any one of these would be decisive:

- a complete MX490 SPI dump;
- a runtime memory dump containing the HTTP server's route/callback table;
- a second official firmware image for differential analysis;
- a verified read-only service/debug interface that exposes symbol or address
  information.

Until one exists, a modified S-record can be generated syntactically but cannot
be called a safe, flashable AirScan firmware.

## Sources

1. Reddit, “First firmware dump of Canon PIXMA TS3451,” July 2026, describing
   Raspberry Pi/flashrom SPI acquisition and the extracted PIXMA runtime:
   <https://www.reddit.com/r/ReverseEngineering/comments/1uxmszf/>
2. HITBSecConf, “ARM Wrestling a Printer: How to Mod Firmware,” 2014:
   <https://archive.conference.hitb.org/hitbsecconf2014kul/sessions/arm-wrestling-a-printer-how-to-mod-firmware/>
3. Synacktiv, `canon-tools` firmware analysis tools:
   <https://github.com/synacktiv/canon-tools>
4. Canon Singapore, “Printer Firmware Updater (Windows) for MX490 series Ver.1.0”:
   <https://asia.canon/en/support/0400755002>
5. Canon, “MX490 series — Firmware update” manual:
   <https://ij.manual.canon/ij/webmanual/Manual/M/MX490%20series/EN/UG/ug_changing_settings1100.html>
