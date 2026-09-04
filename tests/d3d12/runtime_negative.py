"""Run native failure cases serially; never overlap a GPU validation process."""
import json
from pathlib import Path
import subprocess
import sys

root = Path(__file__).resolve().parents[2]
exe = Path(sys.argv[1]).resolve()
for name, args, expected in [
    ("adapter", ["--hardware", "--adapter-index", "99999", "--headless", "--frames", "1"], "UnsupportedAdapter"),
    ("watchdog", ["--warp", "--headless", "--frames", "1000000", "--watchdog-ms", "1"], "WatchdogTimeout"),
]:
    path = root / "artifacts/reports" / f"negative-{name}.json"
    result = subprocess.run([str(exe), *args, "--report", str(path)], capture_output=True, encoding="utf-8", timeout=90)
    assert result.returncode == 1, (name, result.returncode, result.stderr)
    report = json.loads(path.read_text(encoding="utf-8"))
    assert not report["success"] and report["failure_code"] == expected, report
    print(f"PASS {name}: {expected}; HRESULT={report['hresult']}; frames={report['frames']}")
result = subprocess.run([str(exe), "--frames", "-1"], capture_output=True, timeout=10)
assert result.returncode == 2
print("PASS CLI invalid frame count: typed configuration failure")
