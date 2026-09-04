"""Procedural scene quality, pass, timing and alias parity acceptance; serial."""
import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[2]
exe = Path(sys.argv[1]).resolve()
backend = sys.argv[2] if len(sys.argv) > 2 else "warp"
base = root / "artifacts/scene-validation"
base.mkdir(parents=True, exist_ok=True)
directory = Path(tempfile.mkdtemp(prefix=f"{backend}-", dir=base))
required = [
    "InitCulling", "GPUFrustumCulling", "DepthPrepass", "SceneHDR", "BloomExtract",
    "BloomBlurHorizontal", "BloomBlurVertical", "ToneMap", "ReadbackCulling", "Capture", "Present",
]
pixels, reports = [], []
for policy in ("on", "off"):
    command = [str(exe), f"--{backend}", "--headless", "--frames", "12", "--width", "640", "--height", "360", "--scene-seed", "24301",
        "--draw-mode", "gpu", "--aliasing", policy, "--capture", str(directory / f"{policy}.png"), "--rgba", str(directory / f"{policy}.rgba"),
        "--plan", str(directory / f"{policy}-plan.json"), "--report", str(directory / f"{policy}.json")]
    result = subprocess.run(command, capture_output=True, encoding="utf-8", timeout=180)
    assert result.returncode == 0, (result.returncode, result.stdout, result.stderr)
    plan = json.loads((directory / f"{policy}-plan.json").read_text(encoding="utf-8"))
    active = [plan["passes"][p]["name"] for p in plan["order"]]
    assert active == required, active
    resources = {r["name"]: r for r in plan["resources"]}
    assert resources["Depth"]["format"] == 2 and resources["Depth"]["width"] == 640
    assert resources["SceneHDR"]["format"] == 1 and resources["SceneHDR"]["height"] == 360
    for name in ("BloomA", "BloomB", "BloomC"):
        assert resources[name]["format"] == 1 and resources[name]["width"] == 320 and resources[name]["height"] == 180
    assert (len(plan["allocation"]["aliases"]) >= 1) == (policy == "on")
    report = json.loads((directory / f"{policy}.json").read_text(encoding="utf-8"))
    assert report["success"] and report["debug_errors"] == report["debug_warnings"] == report["debug_corruptions"] == 0
    assert report["draw_mode"] == "gpu" and report["cpu_visible_count"] == report["gpu_visible_count"]
    assert report["plan_identity"] == plan["plan_identity"] and report["plan_compile_count"] == 1
    assert report["non_black_fraction"] > 0.20 and report["color_buckets"] >= 32
    assert report["luminance_max"] - report["luminance_min"] >= 50
    assert [t["name"] for t in report["pass_timings"]] == required
    assert all(t["gpu_samples"] == 12 and t["gpu_ms"] >= 0 for t in report["pass_timings"])
    rgba = (directory / f"{policy}.rgba").read_bytes()
    assert len(rgba) == 640 * 360 * 4
    png = (directory / f"{policy}.png").read_bytes()
    assert png[:8] == b"\x89PNG\r\n\x1a\n" and struct.unpack(">II", png[16:24]) == (640, 360)
    pixels.append(rgba); reports.append(report)
assert pixels[0] == pixels[1], "neon scene alias on/off RGBA differs"
assert reports[0]["actual_heap_bytes"] < reports[1]["actual_heap_bytes"]
print(json.dumps({"success": True, "backend": backend, "draw_mode": "gpu", "directory": str(directory.relative_to(root)), "pixel_hash": reports[0]["pixel_hash"],
    "on_bytes": reports[0]["actual_heap_bytes"], "off_bytes": reports[1]["actual_heap_bytes"], "color_buckets": reports[0]["color_buckets"]}))
