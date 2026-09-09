# Custom-firmware build status

The project now has a reproducible offline analysis step:

```sh
python3 firmware/analyze_image.py decoded.bin -o firmware-report.json
```

The report inventories readable firmware strings and records whether the decoded image contains AirScan/eSCL markers. It does not write to a printer or alter the input image.

## Reference package

The official Canon Windows updater was downloaded from Canon's regional support CDN and verified:

```text
fuu_-win-mx490-1_0-ea7.exe
SHA-256: 71fa45d1f3e1da125fa71f03f29fc7c8587bbe9c97ff22e684fb4ce12dcdd134
Target: MX490 series firmware 4.040
```

The original executable and extracted firmware payload are kept outside this repository. The repository contains no Canon binary.

The decoded firmware also exposes Canon's device-specific update URL:

```text
http://gdlp01.c-wss.com/rmds/ij/ijd/ijdupdate/1787.bin
```

That endpoint is currently live and returns a 272-byte opaque binary for USB ID `04A9:1787` (MX490 series). It is update metadata or a manifest, not the firmware image itself. It is now the strongest lead for recovering the current server-delivered revision.

The CDN reports `Last-Modified: Wed, 02 Apr 2025 08:27:33 GMT` and ETag `205519b384407d69c3aaf34b32c0a4d1`. The file has 17 AES-sized blocks and high entropy, so it is consistent with an encrypted manifest rather than a plain XML descriptor. No plaintext firmware URL is present.

The endpoint can be fetched and hashed without connecting to or modifying a printer:

```sh
python3 firmware/fetch_manifest.py /tmp/mx490-1787.bin
```

The manifest can be inventoried locally with:

```sh
python3 firmware/analyze_manifest.py /tmp/mx490-1787.bin \
  -o firmware/1787-analysis.json
```

The current sample is 272 bytes, 16-byte aligned, has no printable strings,
and measures 7.28653 bits of entropy per byte. That makes it consistent with
encrypted metadata, but does not identify the cipher or key.

## Lab-only repack test

The 4.040 S-record stream was modified at one test byte, its record checksum was recalculated, and the repeating XOR container was rebuilt. Decrypting the rebuilt payload reproduced exactly one changed byte at the same offset and preserved the original length. This validates the repackaging mechanics only; it is not an AirScan implementation and must not be flashed.

The current reference image is Canon's 4.040 payload. It confirms the existing DryOS/ITRON, BJNP, Bonjour, HTTP, IPP, and WSD scanner components, but no native AirScan/eSCL implementation. A real CFW would need a new eSCL HTTP service, Bonjour advertisement, scan-job adapter, streaming, cancellation, and update acceptance.

The live device is treated as 4.050. Until its image is obtained, the build cannot claim compatibility with that device. Any generated 4.040 patch remains a laboratory artifact and must not be flashed.

The next implementation milestone is a host-side eSCL adapter that exercises the same scan-job behavior without changing printer firmware. That gives us a working protocol reference while the firmware image gap remains unresolved.

The decoded 4.040 image also contains four length-prefixed zlib components.
They decompress to three 1 MiB blocks and one 443,160-byte block. The block
inventory is reproducible with:

```sh
python3 firmware/extract_zlib_blocks.py decoded-4.040.bin /tmp/mx490-blocks \
  --json firmware/zlib-blocks.json
```

One component contains the live `1787.bin` update URL and the update user
interface strings. This narrows the next reverse-engineering step to the
manifest verification path rather than guessing at a firmware container.

## Comparison workflow

When a verified 4.050 image becomes available, compare decoded images with:

```sh
python3 firmware/compare_images.py decoded-4.040.bin decoded-4.050.bin \
  -o firmware/4.040-vs-4.050.json
```

The comparison records hashes, image lengths, the first changed offset, and the number of changed bytes. It does not assume that a changed region is safe to patch; service-level and update-verification analysis still has to be done separately.
