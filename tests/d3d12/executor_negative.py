"""Serial fail-closed checks for graph, access and capture deadline boundaries."""
import json
from pathlib import Path
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[2]
exe = Path(sys.argv[1]).resolve()
base = root / "artifacts/executor-negative"
base.mkdir(parents=True, exist_ok=True)
directory = Path(tempfile.mkdtemp(dir=base))
assert directory.resolve().is_relative_to(root.resolve())
cases = [
    ("invalid-graph", ["--validation-invalid-graph"], "InvalidGraph"),
    ("undeclared", ["--validation-undeclared"], "UndeclaredAccess"),
    ("capture-timeout", ["--capture-timeout-ms", "0", "--capture", str(directory / "must-not-exist.png")], "CaptureTimeout"),
]
for name, extra, expected in cases:
    path = directory / f"{name}.json"
    result = subprocess.run([str(exe), "--warp", "--headless", "--frames", "4", *extra, "--report", str(path)], capture_output=True, encoding="utf-8", timeout=90)
    assert result.returncode == 1, (name, result.returncode, result.stdout, result.stderr)
    report = json.loads(path.read_text(encoding="utf-8"))
    assert report["failure_code"] == expected and not report["success"], report
    assert report["debug_errors"] == report["debug_warnings"] == report["debug_corruptions"] == 0, report
    print(f"PASS {name}: {expected}")
assert not (directory / "must-not-exist.png").exists()
