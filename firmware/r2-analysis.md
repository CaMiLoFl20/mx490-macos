# ARM cross-reference analysis

The decoded S-record image was loaded into radare2 as ARM code at
`0x04000000`, matching the memory mapping documented for related Canon DryOS
firmware. A full `aaa` pass completed, but the apparent reference to
`tskWSDScanInfoServi...` at `0x042df09e` is classified as data, not executable
code. The surrounding bytes do not disassemble into a coherent function.

This is useful negative evidence: the visible scanner strings are in a data or
compressed/resource region, so direct string cross-references cannot yet locate
the live WSD handler. The next analysis must recover the runtime decompression
or relocation map before handler pointers can be patched.

Reproduction (radare2 6.2.3):

```sh
export DYLD_LIBRARY_PATH=/private/tmp/mx490-fw/r2-install/lib
/private/tmp/mx490-fw/r2-install/bin/r2 -q -a arm -b 32 \
  -m 0x04000000 \
  -c 'aaa; axt @ 0x042df1ac' \
  /private/tmp/mx490-fw/decoded.bin
```
