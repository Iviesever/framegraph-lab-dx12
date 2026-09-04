"""Parse actual executable output and verify canonical semantic hash independently."""
import json
import subprocess
import sys

first = subprocess.check_output([sys.argv[1]], timeout=30).decode("utf-8").strip()
second = subprocess.check_output([sys.argv[1]], timeout=30).decode("utf-8").strip()
assert first == second, "canonical bytes changed across processes"
plan = json.loads(first)
assert plan["schema_version"] == 1
assert plan["resources"][0]["name"] == 'source"\n中'
assert plan["order"] == [0, 1, 2]
assert plan["culled"] == [3]
assert plan["allocation"]["physical_bytes"] == 65536
assert len(plan["allocation"]["aliases"]) == 1
payload = first[:first.rindex(',"plan_identity":')] + "}"
value = 0xCBF29CE484222325
for byte in payload.encode("utf-8"):
    value = ((value ^ byte) * 0x100000001B3) & 0xFFFFFFFFFFFFFFFF
assert plan["plan_identity"] == f"{value:016x}"
print(f"canonical JSON parsed; fresh-process repeat and identity passed: {value:016x}")
