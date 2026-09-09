# MX490 scanning on modern macOS

Feasibility project for restoring Canon PIXMA MX490 series scanning on macOS 26.6.2 (25G83).

Status: feasibility confirmed. A direct network scan succeeded on macOS 26.6.2; the patched AirSane bridge appeared in Image Capture and completed a full scan job through the Apple interface. The bridge and Canon device still appear as separate Bonjour records because they implement different protocols.

## Findings

The inspected Mac runs Apple Silicon. Its installed Canon IJScanner15s component is version 5.0.0 and contains both arm64 and x86_64 executables. A missing Apple Silicon executable is therefore not the explanation; runtime compatibility and scanner connectivity remain unverified.

SANE lists the MX490 series (USB ID 04a9:1787) for USB and Wi-Fi under its pixma backend, but labels support “Untested / Testers needed”. This is a useful starting point, not proof of functionality.

Testing on the attached MX490 found it at a local BJNP scanner endpoint. SANE 1.4.0 acquired a 1240 x 1754 color PNG at 150 dpi over Wi-Fi. Image Capture initially failed against Canon's component with error `-21345`. AirSane exposed a second Bonjour scanner in Image Capture and its overview rendered correctly. Image Capture's output-file behavior should be retested on a clean installation before calling the bridge release-ready.

AirSane exposes SANE scanners through AirScan/eSCL to Image Capture and includes macOS build instructions. Proposed architecture:

Image Capture -> local AirSane service -> SANE pixma backend -> MX490

## Investigation milestones

1. Discover the printer's advertised Bonjour services and resolve its address. Check whether a usable eSCL service exists before concluding that driverless scanning is unavailable.
2. Reproduce the current Image Capture failure and inspect relevant driver loading errors. Distinguish discovery, runtime, and transport failures.
3. Build or install SANE and test MX490 discovery over Wi-Fi and, where available, USB.
4. Acquire a 150 dpi flatbed test scan using a non-sensitive page. Check dimensions, colors, completeness, and cancellation.
5. If the backend works, integrate AirSane and repeat the test through Image Capture. Validate document feeder behavior separately.
6. If the backend fails, investigate the existing open-source implementation and the device protocol, document the specific discrepancy, and implement a targeted fix.

The first targeted fix is now identified: AirSane was advertising the MX490 flatbed as 216 x 355.6 mm because the SANE backend exposes the ADF/legal range. Canon specifies the platen as 216 x 297 mm. A model-specific correction was added to the local AirSane build; its capabilities now report 216 x 297 mm for the flatbed while retaining the feeder range. A second patch makes the bridge publish the user-facing name `Canon MX490 series (AirScan)`.

The local prototype also confirmed that using the printer's original `_scanner._tcp` service type hides the separate entry but routes Image Capture back to Canon's failing endpoint. That experiment was reverted; the working bridge uses `_uscan._tcp`.

A reversible service-type experiment changed the bridge from `_uscan._tcp` to the printer's `_scanner._tcp`. Image Capture then showed a single entry, but connected to Canon's endpoint and reproduced `-21345`; the bridge could not be used. The experiment was reverted. A real single-service solution therefore needs a proxy that owns the Canon endpoint and translates its protocol, rather than a Bonjour rename.

## Proposed GitHub scope

Repository name: mx490-macos

Publish original diagnostic tools, reproducible build instructions, compatibility results, and any necessary patches. Keep raw device addresses, serial numbers, scan images, and local logs outside version control. Honor upstream licenses when distributing or adapting upstream code. Do not include Canon driver binaries.

The current source change is captured in [`airsane-mx490.patch`](airsane-mx490.patch). It applies only to AirSane's scanner naming and MX490 flatbed capability reporting; it does not include vendor software.

## Compatibility results

| Component | Result | Notes |
| --- | --- | --- |
| macOS 26.6.2 (25G83), Apple silicon | Working test environment | SANE and AirSane built successfully |
| MX490 Wi-Fi discovery | Working | SANE found the device at its BJNP scanner endpoint |
| Direct SANE flatbed scan | Working | 150 dpi color scan completed successfully |
| AirSane discovery in Image Capture | Working | Overview image displayed |
| AirSane scan transfer through Image Capture | Working prototype | Full transfer completed; output-file behavior should be retested on a clean installation |
| Canon Image Capture plugin | Fails | Image Capture reports error `-21345` |
| Single `_scanner._tcp` replacement | Failed and reverted | Image Capture routed back to Canon's endpoint |

Run `./setup-and-test.sh` from this directory to reproduce the build. The script uses a separate `work/` directory by default and does not install a system daemon.

First release criterion: a reproducible successful scan on macOS 26.6.2, with the exact scanner model, connection type, backend version, and known limitations documented.

## Sources

- SANE device table: https://www.sane-project.org/sane-mfgs.html
- SANE pixma backend: https://www.sane-project.org/man/sane-pixma.5.html
- AirSane: https://github.com/SimulPiscator/AirSane
- AirSane macOS build: https://github.com/SimulPiscator/AirSane/blob/master/README.macOS.md

Sources reviewed September 9, 2026.
