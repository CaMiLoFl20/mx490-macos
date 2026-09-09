# Mac MX490 updater analysis

The mounted Canon DMG was inspected without starting an update. The application
is a 64-bit x86_64 Mach-O executable:

`Printer Update.app/Contents/MacOS/Printer Update`

The embedded payload is:

`Data/MX490-4_040.dat`

The payload SHA-256 is recorded locally as
`e594858dbe8326eafd461b9fc0caabcd11ddea2dc4893a3ac228f88d81eaf80c`. The
executable contains Objective-C symbols that expose the updater sequence:

- `findUSBDeviceWithProductID:`
- `openUSBPortWithProductID:`
- `getPrinterInformationWithDeviceModel:`
- `sendBJLCommandWithBuffer:length:`
- `sendRomDataBytes:length:`
- `cij_shiftFRomModeWithInformation:`
- `cij_ivecSendFRomCommandWithUSBModel:`
- `cij_sendFRomCommandWithUSBModel:`
- `cij_waitingWhileBusyStateWithDeviceModel:romModel:sendReadModel:productID:`
- `cij_checkDeviceChangeFlushModeWithDeviceModel:romModel:sendReadModel:productID:`

The binary also embeds this Canon command:

```xml
<?xml version="1.0" encoding="utf-8" ?><cmd xmlns:ivec="http://www.canon.com/ns/cmd/2008/07/common/" xmlns:vcn="http://www.canon.com/ns/cmd/2008/07/canon/"><ivec:contents><ivec:operation>VendorCmd</ivec:operation><ivec:param_set servicetype="device"><vcn:ijoperation>FRomUpMode</vcn:ijoperation></ivec:param_set></ivec:contents></cmd>
```

## Implication

The updater appears to enter a Canon firmware-update mode over USB, perform a
device/version exchange, and then send the 4.040 payload in ROM-data chunks. The
named `sendBJLCommand` and `receiveStatusData` methods give us a tractable
target for static disassembly and, if needed, USB traffic capture. This is more
promising than trying to infer the HTTP hook from the firmware image alone.

The updater does not appear to expose a public firmware-read operation by name;
the visible ROM method is a write path. Therefore the immediate safe objective
is to recover the **read-only identity/status exchange** and the exact transition
command, not to invoke the write path or force a downgrade from 4.050.

## Next safe analysis

1. Disassemble the named methods and identify the USB request direction, request
   type, endpoint, and response lengths.
2. Run the updater only through device detection/version display and capture the
   USB exchange if macOS exposes it through IOKit logs or a USB analyzer.
3. Compare the reported 4.050 version bytes with the updater's 4.040 metadata.
4. Do not call `sendRomDataBytes:` or click Start; those are write operations.

This evidence does not yet provide a flash dump or a safe custom-firmware hook,
but it narrows the acquisition problem to a small, named set of updater methods.
