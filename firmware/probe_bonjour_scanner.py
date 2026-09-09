#!/usr/bin/env python3
"""Resolve Canon's Bonjour scanner service without sending scan commands."""
import argparse, json, re, subprocess

def main():
    ap=argparse.ArgumentParser(); ap.add_argument('--name',default='Canon MX490 series'); ap.add_argument('-o','--output',required=True); a=ap.parse_args()
    cmd=['dns-sd','-L',a.name,'_scanner._tcp','local.']
    proc=subprocess.Popen(cmd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
    try:
        raw,_=proc.communicate(timeout=3)
    except subprocess.TimeoutExpired:
        proc.terminate(); raw,_=proc.communicate()
    text=raw.decode('utf-8','replace')

    m=re.search(r'can be reached at ([^: ]+):([0-9]+)',text)
    rec={'service_type':'_scanner._tcp.local.','instance_name':a.name,'raw_status':text,'resolved':bool(m)}
    if m:
        rec['host']=m.group(1); rec['port']=int(m.group(2)); rec['host_redacted']=True
        rec['raw_status']=re.sub(r'(?<=at) [^: ]+', ' <redacted-host>', text)
    txt=re.findall(r'\b(?:txtvers|ty|adminurl|mfg|mdl|mac|UUID|scannerAvailable)=[^\s]+',text)
    rec['txt']=[re.sub(r'(adminurl=)[^\s]+',r'\1<redacted>',x) for x in txt]
    with open(a.output,'w') as f: json.dump(rec,f,indent=2); f.write('\n')
    print(json.dumps({k:rec[k] for k in ('service_type','instance_name','resolved','port','txt') if k in rec},indent=2))
    return 0 if m else 1
if __name__=='__main__': raise SystemExit(main())
