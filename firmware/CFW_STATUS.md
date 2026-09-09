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

The raw decoded image has a real scanner-service anchor: the printable task
identifier `tskWSDScanInfoServi...` at offset `0x2df1ac`, plus `WSDBasicIO.cpp`,
`WSDManager`, `ScanJobItem`, `[ScanParam]`, and `[PageScan Par...]` strings.
That establishes that the image already has a structured WSD scanner task and
scan-job data model. The repository now includes `inventory_services.py` to
reproduce this service/string inventory against any later 4.050 dump.

The same image also contains an internal scan-job message schema around offset
`0x4996b4`: `X-CISSE-SCAN`, `<idJob>`, `jobStatus>submitted`, `headerInfo`,
`vcToken`, and `judgedDocumentType`. This is a stronger integration lead than
inventing a new scanner transport: a native eSCL handler could translate its
`/eSCL/ScannerStatus`, `ScanJobs`, and image-stream requests into this existing
scan-job machinery if the corresponding handler functions are recovered.

The route inventory also finds two `/wsd/` literals (`0x278977` and
`0x29738f`) and the existing CGI route vocabulary. These are likely entries in
the firmware's HTTP dispatch tables. The next patching target is therefore the
dispatch table and its handler pointers, followed by a minimal eSCL status/job
adapter; the scanner engine itself appears to be present already.

The repository now contains `escl_adapter.c/.h`, a platform-neutral eSCL
handler boundary with capabilities, status, job submission, image streaming,
and cancellation routes. It compiles under C99 with warnings treated as
errors, and `test_escl_adapter.c` exercises the route flow. This is an
integration component, not a flash image: its callbacks still need to be
connected to the MX490 `X-CISSE-SCAN` functions recovered from the firmware.

An ARM cross-reference pass using radare2 and the related-Canon `0x04000000`
memory mapping found that the visible WSD task string is in a data/resource
region rather than directly in executable code. The result and exact command
are recorded in `r2-analysis.md`; runtime decompression/relocation recovery is
now the prerequisite for identifying handler pointers.

Static inspection shows that the same component includes TLS cipher tables,
AES-GCM, SHA, and RSA references. That is evidence that the device has the
cryptographic machinery needed to validate update metadata, but it is not a
recoverable signing key. Replacing an AirScan service would still require a
verified image that the bootloader accepts, plus a correct runtime load address
and memory budget for the new HTTP and scan-job code.

There is a useful precedent for this platform family: Synacktiv documented
older DryOS Canon PIXMA models using the same hardcoded USB-product update URL,
XOR-wrapped S-record firmware, and ARM runtime layout. Their report says those
older updates were not signed and describes the HTTP/BJNP task structure. That
raises the feasibility of a research build, but it does not prove that the
MX490's 4.050 boot path accepts a modified image. The report is a reference for
analysis only: <https://www.synacktiv.com/sites/default/files/2021-06/thcon2021_canon_printer.pdf>.

## Comparison workflow

When a verified 4.050 image becomes available, compare decoded images with:

```sh
python3 firmware/compare_images.py decoded-4.040.bin decoded-4.050.bin \
  -o firmware/4.040-vs-4.050.json
```

The comparison records hashes, image lengths, the first changed offset, and the number of changed bytes. It does not assume that a changed region is safe to patch; service-level and update-verification analysis still has to be done separately.

The decrypted container begins and ends with Canon-specific `SF` wrapper
records around standard Motorola S-records. `inventory_srecords.py` now
validates every standard record and reports the load-address range and start
record while preserving those wrapper records as metadata. On the 4.040
reference stream it validates 717,658 data records spanning
`0xF0020000`–`0xF0FF0000` with start address `0xF0FF0000`. This establishes
the image's address space for later code and decompressor analysis; it does
not identify a safe patch point or prove that a modified image will boot.

The boot-prefix vector scan is reproducible with:

```sh
python3 firmware/analyze_boot_vectors.py decoded-4.040.bin \
  -o firmware/boot-vectors-4.040.json
```

It finds four ARM `ldr pc, [pc, #imm]` stubs in the first `0x5004` bytes.
Two load low addresses `0x001363AC` and `0x00000148`, one loads the literal
`0x003A302F`, and one branches back into the image at `0xF00200BC`. This
supports a relocation or ROM-call transition before the compressed application
components become executable. The scan is descriptive only; it does not infer
that any low address is safe to call or patch.

The decoded image also embeds Canon's catalog endpoints at
`dtv-p.c-ij.com/sdata/struct01/version.bin` and `sdata.bin`. Their current
responses are recorded without storing the binary contents in the repository:

```sh
python3 firmware/fetch_catalog.py -o firmware/catalog-YYYY-MM-DD.json
```

The current two-byte catalog value is `08 00`. The 64 KiB `sdata.bin` object
decrypts with the published `dec_sdata` table into a CA trust store; it does
not contain the MX490 firmware or a signing key. The MX490-specific
`1787.bin` object remains a separate 272-byte opaque manifest, while the
legacy `1787.xml` URL returns HTTP 404. These observations narrow the update
path but do not recover firmware 4.050.

The live printer's installed version is now independently confirmed through a
read-only BJNP `GetId` request (UDP 8611). The response identifies `MX490
series` and reports `VER:4.050`, along with the expected Canon printer command
set. Reproduce this locally with:

```sh
python3 firmware/query_bjnp_identity.py PRINTER_IP
```

The repository does not store the printer's IP address or full identity
response. This confirms the comparison target is 4.050, but it does not expose
flash contents or authorize a firmware write.

A direct read-only service probe against the live 4.050 printer returns HTTP
404 for `/eSCL/ScannerCapabilities`, `/eSCL/ScannerStatus`, and `/wsd/` over
both HTTP and HTTPS. The update CGI is present but authentication-protected
(HTTP 401). Reproduce the route check without storing the printer address or
response body with:

```sh
python3 firmware/probe_live_services.py PRINTER_IP
```

This is the baseline a native eSCL patch must change: the existing BJNP/WSD
scanner remains reachable, but no eSCL HTTP endpoint is currently registered.

The compression scan is now automated with `firmware/scan_compression.py`. On
4.040 it finds exactly the four valid zlib streams listed above and finds zero
instances of the related-model Canon LZ decompressor signature
`70 b5 05 4c 05 48 06 49 45 1a 0e 46 2a 46 31 46 20 46 ff`. This means the
known MG6450-style unpacker cannot be reused as-is for MX490. It does not prove
that no other decompressor exists, but it removes the only known direct route
to recovering executable application code from this image. The resulting
machine-readable checkpoint is `firmware/compression-4.040.json`.

A further ARM disassembly pass located the embedded zlib `inflate` runtime at
file offset `0x2cb8` (runtime `0xF0022CB8` with the verified image mapping),
ending near `0x3164`. Its bit-buffer refill, canonical Huffman table lookups,
literal output loops, length-distance copies, and nearby zlib error strings are
consistent with a standard `inflate` implementation. The finding is recorded
in `firmware/zlib-runtime-4.040.json`. This is a loader/runtime anchor for
future call-site recovery; it is not evidence that a modified image can be
accepted or booted by the printer.

A full ARM branch scan found one direct caller of that inflate core, at file
offset `0x1e48` (runtime `0xF0021E48`). The caller passes stream/output state
through the `r4`/`r5` structures, checks the returned status, and resumes a
larger state machine. This is the first concrete loader callsite recovered and
is recorded in `firmware/zlib-callsite-4.040.json`; the next task is to recover
its state structure and stream descriptors rather than guessing at patch
locations.

The surrounding code identifies a zlib-style wrapper at file offset `0x2ac4`
(runtime `0xF0022AC4`). It allocates a `0x18000`-byte work area and a
`0x370`-byte inflate state object, calls the main state machine at
`0xF002133C`, and handles the expected zlib return values. This is recorded in
`firmware/zlib-wrapper-4.040.json`. The wrapper is likely reached through a
relocation table or function pointer because no direct static ARM `BL` caller
is present in the image.

The verified S-record intervals also expose several erased/reserved ranges
inside the mapped flash space. The largest are runtime `0xF0B10458–0xF0C81000`
(about 1.45 MiB) and `0xF0CA2198–0xF0FEFFFF` (about 3.30 MiB), with smaller
holes near `0xF024DC7D`. These ranges could theoretically hold a native eSCL
handler, but only after proving the bootloader accepts records there, the
runtime maps them executable, and the image integrity checks remain valid. The
inventory is recorded in `firmware/free-space-4.040.json`; nothing has been
written into those ranges.
