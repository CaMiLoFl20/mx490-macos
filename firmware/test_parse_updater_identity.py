import importlib.util
from pathlib import Path

spec = importlib.util.spec_from_file_location("parser", Path(__file__).with_name("parse_updater_identity.py"))
parser = importlib.util.module_from_spec(spec)
spec.loader.exec_module(parser)

def test_identity_record():
    data = b"xx;MDL:MX490 series;VER:4.050;DES:device;STA:20;CMD:ready;yy"
    assert parser.parse_record(data) == {
        "MDL": "MX490 series", "VER": "4.050", "DES": "device",
        "STA": "20", "CMD": "ready"
    }

if __name__ == "__main__":
    test_identity_record()
