# Protocol references for MX490 integration

These references inform the firmware research track but do not prove that any
external protocol sequence is accepted by the MX490 4.050 firmware.

- Canon's MX490 manual documents a `WSD scan from dev.` network setting and
  says enabling it transfers scans to a computer using WSD:
  [Canon MX490 LAN settings](https://ij.manual.canon/ij/webmanual/Manual/S/MX490%20series/EN/UG/ug_changing_settings0300.html).
- The archived `libmfp2-canon` backend explicitly lists **MX490 series** among
  the Canon models supported by its newer proprietary scanner protocol. It is
  GPL-2.0 and must be treated as a reference rather than copied into this
  project without license review:
  [libmfp2-canon model list](https://github.com/ThierryHFR/libmfp2-canon).
- `sane-airscan` lists **Canon MX470 series** as WSD-capable, but does not list
  MX490 in its tested table. That is evidence that nearby Canon families can
  expose WSD, not evidence that MX490 exposes eSCL:
  [sane-airscan compatibility](https://github.com/alexpevzner/sane-airscan).
- The public CHMP reverse-engineering project documents a detailed
  POST-then-GET scan sequence for the G3010 family, including `0xd820`,
  `0xd920`, `0xda20`, and `0xd420`. Its README explicitly scopes the result to
  G3010, so this repository only uses it as a serializer design reference:
  [G3010 CHMP scanner](https://github.com/wailun42/canon-g3010-chmp-scanner).

The MX490-specific implementation must still validate its handler and scan
parameters against the printer before any firmware write is considered.
