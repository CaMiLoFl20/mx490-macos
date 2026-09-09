# Runtime decompression investigation

The Synacktiv tooling for related PIXMA models uses Unicorn to call a small
LZ-style decompressor at a model-specific address, with output relocated into
RAM. Unicorn 2.1.4 is available in the local analysis environment and the
related script was reviewed.

The published MG6450 call site (`0x04220058`) does not contain ARM code at the
corresponding MX490 offset, so copying that address would be invalid. A direct
radare2 analysis of the MX490 image also classified the visible WSD strings as
data. The MX490 decompressor call site and dictionary/output addresses must be
recovered from this model's boot code before runtime handler pointers can be
resolved.

This rules out a safe blind port of the MG6450 decompression script. The next
candidate is to identify the MX490 bootloader's memory-copy/decompression call
by matching its ARM instruction pattern and output-size constants.

`find_constants.py` records candidate occurrences of those constants in a
repeatable way while that search is in progress:

```sh
python3 firmware/find_constants.py decoded-4.040.bin \
  -o firmware/decompress-constants.json
```
