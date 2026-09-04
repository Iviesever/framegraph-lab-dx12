"""Validate the fixed-graph Core benchmark schema and invariants."""

import json
from pathlib import Path
import subprocess
import sys


executable = Path(sys.argv[1]).resolve()
result = subprocess.run(
    [str(executable), "--samples", "7", "--iterations", "50"],
    capture_output=True,
    encoding="utf-8",
    timeout=120,
)
assert result.returncode == 0, (result.returncode, result.stdout, result.stderr)
data = json.loads(result.stdout)
assert data["schema_version"] == 1
assert data["graph"] == {"resources": 64, "passes": 192, "usages": 192}
assert len(data["plan_identity"]) == 16
assert data["samples"] == 7 and data["iterations_per_sample"] == 50
for name in ("compile", "allocation_alias_on", "allocation_alias_off", "barriers_alias_on", "barriers_alias_off"):
    measurement = data["measurements"][name]
    assert len(measurement["raw_us"]) == 7
    assert measurement["median_us"] > 0
assert data["plans"]["alias_on"]["physical_bytes"] < data["plans"]["alias_off"]["physical_bytes"]
assert data["plans"]["alias_on"]["aliases"] > 0
assert data["plans"]["alias_on"]["uav_barriers"] == data["plans"]["alias_off"]["uav_barriers"] > 0
assert data["plans"]["alias_on"]["transitions"] == data["plans"]["alias_off"]["transitions"] > 0
assert data["checksum"] != 0
print(json.dumps({"success": True, "plan_identity": data["plan_identity"], "measurements": data["measurements"], "plans": data["plans"]}))
