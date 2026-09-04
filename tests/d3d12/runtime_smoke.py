"""Bounded subprocess acceptance for a real native runtime report."""
import json
from pathlib import Path
import subprocess
import sys

exe = Path(sys.argv[1]).resolve()
backend = sys.argv[2] if len(sys.argv) > 2 else "warp"
root = Path(__file__).resolve().parents[2]
report = root / "artifacts/reports" / f"runtime-{backend}.json"
report.parent.mkdir(parents=True, exist_ok=True)
command = [str(exe), f"--{backend}", "--headless", "--frames", "24", "--width", "640", "--height", "360", "--report", str(report)]
result = subprocess.run(command, capture_output=True, text=True, encoding="utf-8", timeout=90)
print(result.stdout)
print(result.stderr, file=sys.stderr)
assert result.returncode == 0, f"native runtime failed with exit {result.returncode}"
data = json.loads(report.read_text(encoding="utf-8"))
assert data["success"] and data["frames"] == 24
assert data["debug_enabled"] and data["adapter"]
assert data["backend"] == backend
assert data["debug_errors"] == data["debug_warnings"] == data["debug_corruptions"] == 0
assert data["frames_in_flight"] == 3
assert data["width"] == 640 and data["height"] == 360
print(f"native {backend} clear/present and bounded headless smoke passed")
