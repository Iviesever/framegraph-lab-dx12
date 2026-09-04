"""Repeated resize/minimize/restore validation on one selected adapter."""

import json
from pathlib import Path
import subprocess
import sys
import tempfile


root = Path(__file__).resolve().parents[2]
executable = Path(sys.argv[1]).resolve()
backend = sys.argv[2] if len(sys.argv) > 2 else "warp"
base = root / "artifacts" / "reliability"
base.mkdir(parents=True, exist_ok=True)
directory = Path(tempfile.mkdtemp(prefix=f"{backend}-", dir=base))
report_path = directory / "resize.json"
result = subprocess.run(
    [
        str(executable),
        f"--{backend}",
        "--headless",
        "--frames",
        "100",
        "--width",
        "1280",
        "--height",
        "720",
        "--scene-seed",
        "24301",
        "--draw-mode",
        "gpu",
        "--resize-stress",
        "--report",
        str(report_path),
    ],
    capture_output=True,
    encoding="utf-8",
    timeout=180,
)
assert result.returncode == 0, (result.returncode, result.stdout, result.stderr)
report = json.loads(report_path.read_text(encoding="utf-8"))
assert report["success"] and report["frames"] == 100
assert report["resize_count"] >= 15, report["resize_count"]
assert report["minimize_count"] >= 5 and report["restore_count"] >= 5
assert report["plan_compile_count"] == report["resize_count"] + 1
assert report["cpu_visible_count"] == report["gpu_visible_count"]
assert report["debug_errors"] == report["debug_warnings"] == report["debug_corruptions"] == 0
assert report["width"] == 1280 and report["height"] == 720
assert report["non_black_fraction"] > 0.2 and report["color_buckets"] >= 64
print(
    json.dumps(
        {
            "success": True,
            "backend": backend,
            "directory": str(directory.relative_to(root)),
            "resizes": report["resize_count"],
            "minimizes": report["minimize_count"],
            "restores": report["restore_count"],
            "plan_compiles": report["plan_compile_count"],
            "visible": report["gpu_visible_count"],
            "pixel_hash": report["pixel_hash"],
        }
    )
)
