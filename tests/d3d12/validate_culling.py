"""CPU direct versus GPU ExecuteIndirect culling parity on one fixed camera frame."""

import json
from pathlib import Path
import subprocess
import sys
import tempfile


root = Path(__file__).resolve().parents[2]
exe = Path(sys.argv[1]).resolve()
backend = sys.argv[2] if len(sys.argv) > 2 else "warp"
base = root / "artifacts/culling-validation"
base.mkdir(parents=True, exist_ok=True)
directory = Path(tempfile.mkdtemp(prefix=f"{backend}-", dir=base))
expected_passes = [
    "InitCulling",
    "GPUFrustumCulling",
    "DepthPrepass",
    "SceneHDR",
    "BloomExtract",
    "BloomBlurHorizontal",
    "BloomBlurVertical",
    "ToneMap",
    "ReadbackCulling",
    "Capture",
    "Present",
]
pixels = []
reports = []

for mode in ("gpu", "cpu"):
    command = [
        str(exe),
        f"--{backend}",
        "--headless",
        "--frames",
        "1",
        "--width",
        "640",
        "--height",
        "360",
        "--scene-seed",
        "24301",
        "--draw-mode",
        mode,
        "--aliasing",
        "on",
        "--capture",
        str(directory / f"{mode}.png"),
        "--rgba",
        str(directory / f"{mode}.rgba"),
        "--plan",
        str(directory / f"{mode}-plan.json"),
        "--report",
        str(directory / f"{mode}.json"),
    ]
    result = subprocess.run(command, capture_output=True, encoding="utf-8", timeout=120)
    assert result.returncode == 0, (result.returncode, result.stdout, result.stderr)
    report = json.loads((directory / f"{mode}.json").read_text(encoding="utf-8"))
    plan = json.loads((directory / f"{mode}-plan.json").read_text(encoding="utf-8"))
    names = [plan["passes"][index]["name"] for index in plan["order"]]
    assert names == expected_passes, names
    resources = {resource["name"] for resource in plan["resources"]}
    assert {"VisibleInstances", "IndirectArgs", "CullReadback"} <= resources
    assert plan["barriers"]["uav_count"] >= 2
    assert any(
        barrier["after"] == 9
        for pass_barriers in plan["barriers"]["passes"]
        for barrier in pass_barriers["before"]
        if barrier["kind"] == 0
    )
    assert report["success"] and report["draw_mode"] == mode and report["input_instance_count"] == 160
    assert report["gpu_visible_count"] == report["cpu_visible_count"] and 0 < report["gpu_visible_count"] < 160
    assert report["executed_uav_barriers"] >= 2
    assert report["debug_errors"] == report["debug_warnings"] == report["debug_corruptions"] == 0
    assert all(timing["gpu_samples"] == 1 for timing in report["pass_timings"])
    pixels.append((directory / f"{mode}.rgba").read_bytes())
    reports.append(report)

assert pixels[0] == pixels[1], "GPU indirect and CPU direct pixels differ"
assert reports[0]["plan_identity"] == reports[1]["plan_identity"]
print(
    json.dumps(
        {
            "success": True,
            "backend": backend,
            "directory": str(directory.relative_to(root)),
            "visible": reports[0]["gpu_visible_count"],
            "pixel_hash": reports[0]["pixel_hash"],
            "plan_identity": reports[0]["plan_identity"],
        }
    )
)
