"""Real GPU alias parity, outputs and plan-consumption acceptance; serial only."""
import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[2]
exe = Path(sys.argv[1]).resolve()
backend = sys.argv[2] if len(sys.argv) > 2 else "warp"
base = root / "artifacts/executor-validation"
base.mkdir(parents=True, exist_ok=True)
directory = Path(tempfile.mkdtemp(prefix=f"{backend}-", dir=base))
assert directory.resolve().is_relative_to(root.resolve())
reports = []
pixels = []
for policy in ("on", "off"):
    prefix = directory / policy
    command = [str(exe), f"--{backend}", "--headless", "--frames", "8", "--width", "320", "--height", "180",
        "--scene-seed", "24301", "--aliasing", policy, "--capture", str(prefix.with_suffix(".png")),
        "--rgba", str(prefix.with_suffix(".rgba")), "--plan", str(directory / f"{policy}-plan.json"),
        "--report", str(prefix.with_suffix(".json"))]
    result = subprocess.run(command, capture_output=True, encoding="utf-8", timeout=120)
    assert result.returncode == 0, (command, result.returncode, result.stdout, result.stderr)
    report = json.loads(prefix.with_suffix(".json").read_text(encoding="utf-8"))
    assert report.get("placed_resource_count", 0) >= 2, "no actual placed-resource execution reported"
    assert report["actual_heap_bytes"] == report["planned_heap_bytes"]
    assert report["debug_enabled"] and report["debug_errors"] == report["debug_warnings"] == report["debug_corruptions"] == 0
    assert report["executed_alias_barriers"] > 0 and report["executed_uav_barriers"] > 0
    assert report["plan_compile_count"] == 1
    plan = json.loads((directory / f"{policy}-plan.json").read_text(encoding="utf-8"))
    assert plan["plan_identity"] == report["plan_identity"]
    assert len(plan["allocation"]["aliases"]) > 0 if policy == "on" else not plan["allocation"]["aliases"]
    rgba = prefix.with_suffix(".rgba").read_bytes()
    assert len(rgba) == 320 * 180 * 4 and any(rgba[0::4]), "empty or wrong-size RGBA"
    png = prefix.with_suffix(".png").read_bytes()
    assert png[:8] == b"\x89PNG\r\n\x1a\n" and struct.unpack(">II", png[16:24]) == (320, 180)
    assert report["pixel_hash"] and report["non_black_fraction"] > 0.01
    assert len(report["pass_timings"]) == len(plan["order"])
    assert all(t["gpu_samples"] > 0 and t["gpu_ms"] >= 0 for t in report["pass_timings"])
    reports.append(report)
    pixels.append(rgba)
assert pixels[0] == pixels[1], "alias on/off RGBA differs"
assert reports[0]["actual_heap_bytes"] < reports[1]["actual_heap_bytes"]
print(json.dumps({"success": True, "backend": backend, "directory": str(directory.relative_to(root)),
    "pixel_hash": reports[0]["pixel_hash"], "on_bytes": reports[0]["actual_heap_bytes"], "off_bytes": reports[1]["actual_heap_bytes"]}))
