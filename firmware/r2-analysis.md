# ARM cross-reference analysis

The verified S-record inventory establishes the MX490 file mapping as
`0xF0020000 + file_offset`; the earlier `0x04000000` mapping was a related
model hypothesis and was wrong for this image. Loading the image at
`0xF0020000` shows coherent ARM boot code at the entry point and a mixed ARM /
Thumb startup sequence. A full analysis still finds the apparent reference to
`tskWSDScanInfoServi...` (file offset `0x2df1ac`, runtime address
`0xF02FF1AC`) classified as data, not executable code. The surrounding bytes
do not disassemble into a coherent function.

The boot vector area at file offset `0x0794` contains indirect loads to the
low addresses `0x001363ac` and `0x00000148`, rather than pointers into the
`0xF0020000` file range. This is evidence of a later relocation or runtime
mapping step. It explains why a raw-image string cross-reference cannot yet
locate the live WSD handler.

This is useful negative evidence: the visible scanner strings are in a data or
compressed/resource region, so direct string cross-references cannot yet locate
the live WSD handler. The next analysis must recover the runtime decompression
or relocation map before handler pointers can be patched.

Reproduction (radare2 6.2.3):

```sh
export DYLD_LIBRARY_PATH=/private/tmp/mx490-fw/r2-install/lib
/private/tmp/mx490-fw/r2-install/bin/r2 -q -a arm -b 32 \
  -m 0xF0020000 \
  -c 'aaa; axt @ 0xF02FF1AC' \
  /private/tmp/mx490-fw/decoded.bin
```
