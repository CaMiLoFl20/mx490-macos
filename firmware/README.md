# Firmware research track

This directory documents offline analysis of the Canon PIXMA MX490 firmware format.

## Current baseline

- The live printer is being treated as firmware **4.050**, based on the user's report that it has the latest installed update.
- Canon's publicly downloadable MX490 updater is labelled **4.040**. It is a comparison artifact only.
- No firmware is written to the printer by this project.

## What has been verified

The official 4.040 updater contains an encrypted Canon S-record payload. The payload can be decrypted and decoded for offline inspection. The decoded image contains Canon DryOS/ITRON, BJNP, Bonjour, HTTP, IPP, and WSD scanning components. It contains no observed AirScan/eSCL implementation.

The encrypted container can be repacked in a scratch copy and its internal checksums recalculated. That proves the file format is understandable; it does not prove that the printer accepts modified firmware. A 4.040-derived image must not be flashed to a printer running 4.050.

## Build target

The firmware track will first produce a reproducible **analysis build**:

1. Extract and hash the official 4.040 package.
2. Decrypt and decode its S-record image.
3. Inventory scanner, Bonjour, HTTP, update, and verification code paths.
4. Compare those findings with the live 4.050 device through read-only network behavior.
5. Only consider a firmware patch if a genuine 4.050 image and a recovery method become available.

The supported user-facing solution remains the SANE `pixma` backend behind AirSane. Firmware analysis is a research track and is not required for the AirScan bridge.
