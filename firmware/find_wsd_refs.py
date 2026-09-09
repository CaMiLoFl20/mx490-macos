#!/usr/bin/env python3
"""Find literal ARM/Thumb pointers to WSD-related strings in a decoded image."""
import argparse, json, hashlib
from pathlib import Path

BASE = 0xF0020000
NEEDLES = (b"WSDBasicIO", b"tskWSDScanInfoServi", b"WSDManager", b"ScanJobItem", b"WSDSSF")

def main():
    ap=argparse.ArgumentParser(); ap.add_argument('image',type=Path); ap.add_argument('-o','--output',type=Path,required=True); a=ap.parse_args()
    b=a.image.read_bytes(); refs=[]; strings=[]
    for needle in NEEDLES:
        start=0
        while True:
            off=b.find(needle,start)
            if off<0: break
            strings.append({'needle':needle.decode(), 'file_offset':off, 'runtime':hex(BASE+off)})
            vals={ (BASE+off).to_bytes(4,'little'), ((BASE+off)|1).to_bytes(4,'little') }
            for val in vals:
                pos=0
                while True:
                    pos=b.find(val,pos)
                    if pos<0: break
                    refs.append({'string':needle.decode(),'string_offset':off,'pointer_file_offset':pos,'pointer_runtime':hex(BASE+pos),'encoding':val.hex()}); pos+=1
            start=off+1
    out={'image':str(a.image),'size':len(b),'sha256':hashlib.sha256(b).hexdigest(),'mapping':'runtime = 0xF0020000 + file_offset','strings':strings,'literal_pointer_references':refs,'direct_reference_count':len(refs),'interpretation':'No direct literal ARM/Thumb pointer references were found.' if not refs else 'Literal references exist and require disassembly validation.'}
    a.output.write_text(json.dumps(out,indent=2)+'\n')

if __name__=='__main__': main()
