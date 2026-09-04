"""Run serial same-adapter aliasing comparisons and retain every raw report."""

import argparse
import json
from pathlib import Path
import statistics
import subprocess
import tempfile


parser = argparse.ArgumentParser()
parser.add_argument("executable", type=Path)
parser.add_argument("--backend", choices=("hardware", "warp"), default="hardware")
parser.add_argument("--samples", type=int, default=7)
parser.add_argument("--frames", type=int, default=240)
parser.add_argument("--width", type=int, default=1280)
parser.add_argument("--height", type=int, default=720)
parser.add_argument("--seed", type=int, default=24301)
parser.add_argument("--expected-sha")
parser.add_argument("--output", type=Path, required=True)
args = parser.parse_args()
if not 3 <= args.samples <= 31 or not 1 <= args.frames <= 10000:
    parser.error("samples must be [3,31] and frames [1,10000]")
if not 64 <= args.width <= 8192 or not 64 <= args.height <= 8192:
    parser.error("dimensions must be [64,8192]")

root = Path(__file__).resolve().parents[1]
executable = args.executable.resolve()
base = root / "artifacts" / "benchmarks" / "runtime-raw"
base.mkdir(parents=True, exist_ok=True)
directory = Path(tempfile.mkdtemp(prefix=f"{args.backend}-", dir=base))
records = {"on": [], "off": []}
fixed_environment = None

for sample in range(args.samples):
    pair_hashes = []
    for policy in ("on", "off"):
        report_path = directory / f"sample-{sample:02d}-{policy}.json"
        command = [
            str(executable),
            f"--{args.backend}",
            "--headless",
            "--frames",
            str(args.frames),
            "--width",
            str(args.width),
            "--height",
            str(args.height),
            "--scene-seed",
            str(args.seed),
            "--draw-mode",
            "gpu",
            "--aliasing",
            policy,
            "--report",
            str(report_path),
        ]
        result = subprocess.run(command, capture_output=True, encoding="utf-8", timeout=300)
        assert result.returncode == 0, (command, result.returncode, result.stdout, result.stderr)
        report = json.loads(report_path.read_text(encoding="utf-8"))
        assert report["success"] and report["frames"] == args.frames
        assert report["draw_mode"] == "gpu" and report["input_instance_count"] == 160
        assert report["cpu_visible_count"] == report["gpu_visible_count"]
        assert report["debug_errors"] == report["debug_warnings"] == report["debug_corruptions"] == 0
        if args.expected_sha:
            assert report["source_clean"] and report["git_sha"] == args.expected_sha
        environment = {
            "git_sha": report["git_sha"],
            "source_clean": report["source_clean"],
            "backend": report["backend"],
            "adapter": report["adapter"],
            "driver": report["driver"],
            "vendor_id": report["vendor_id"],
            "device_id": report["device_id"],
            "feature_level": report["feature_level"],
            "debug_enabled": report["debug_enabled"],
            "width": report["width"],
            "height": report["height"],
            "frames": report["frames"],
            "seed": args.seed,
            "draw_mode": report["draw_mode"],
        }
        if fixed_environment is None:
            fixed_environment = environment
        else:
            assert environment == fixed_environment
        pass_gpu = {timing["name"]: timing["gpu_ms"] for timing in report["pass_timings"]}
        pass_cpu = {timing["name"]: timing["cpu_record_ms"] for timing in report["pass_timings"]}
        excluded = {"ReadbackCulling", "Capture", "Present"}
        record = {
            "sample": sample,
            "policy": policy,
            "report": str(report_path.relative_to(root)),
            "plan_identity": report["plan_identity"],
            "pixel_hash": report["pixel_hash"],
            "visible_count": report["gpu_visible_count"],
            "logical_bytes": report["logical_bytes"],
            "committed_bytes": report["committed_bytes"],
            "actual_heap_bytes": report["actual_heap_bytes"],
            "saved_bytes": report["saved_bytes"],
            "compile_ms": report["compile_ms"],
            "gpu_total_ms": sum(pass_gpu.values()),
            "gpu_graph_work_ms": sum(value for name, value in pass_gpu.items() if name not in excluded),
            "cpu_record_total_ms": sum(pass_cpu.values()),
            "cpu_record_graph_work_ms": sum(value for name, value in pass_cpu.items() if name not in excluded),
            "transition_count": report["transition_count"],
            "uav_count": report["uav_count"],
            "aliasing_count": report["aliasing_count"],
            "heap_count": report["heap_count"],
            "placed_resource_count": report["placed_resource_count"],
            "alias_reuse_events": report["alias_reuse_events"],
            "executed_transitions": report["executed_transitions"],
            "executed_uav_barriers": report["executed_uav_barriers"],
            "executed_alias_barriers": report["executed_alias_barriers"],
            "pass_gpu_ms": pass_gpu,
            "pass_cpu_record_ms": pass_cpu,
        }
        records[policy].append(record)
        pair_hashes.append(report["pixel_hash"])
    assert pair_hashes[0] == pair_hashes[1], f"sample {sample} alias pixels differ"

median_fields = (
    "logical_bytes",
    "committed_bytes",
    "actual_heap_bytes",
    "saved_bytes",
    "compile_ms",
    "gpu_total_ms",
    "gpu_graph_work_ms",
    "cpu_record_total_ms",
    "cpu_record_graph_work_ms",
    "transition_count",
    "uav_count",
    "aliasing_count",
    "heap_count",
    "placed_resource_count",
    "alias_reuse_events",
    "executed_transitions",
    "executed_uav_barriers",
    "executed_alias_barriers",
)
medians = {
    policy: {field: statistics.median(record[field] for record in policy_records) for field in median_fields}
    for policy, policy_records in records.items()
}
comparison = {
    "physical_bytes_reduced": medians["off"]["actual_heap_bytes"] - medians["on"]["actual_heap_bytes"],
    "physical_reduction_percent": 100
    * (medians["off"]["actual_heap_bytes"] - medians["on"]["actual_heap_bytes"])
    / medians["off"]["actual_heap_bytes"],
    "gpu_graph_work_delta_ms": medians["on"]["gpu_graph_work_ms"] - medians["off"]["gpu_graph_work_ms"],
    "cpu_record_graph_work_delta_ms": medians["on"]["cpu_record_graph_work_ms"]
    - medians["off"]["cpu_record_graph_work_ms"],
}
output = {
    "schema_version": 1,
    "environment": fixed_environment,
    "samples_per_policy": args.samples,
    "execution_order": "paired alias-on then alias-off, serial",
    "raw": records,
    "medians": medians,
    "comparison": comparison,
    "limitations": "local observation; no cross-machine SLA",
}
args.output.parent.mkdir(parents=True, exist_ok=True)
args.output.write_text(json.dumps(output, indent=2) + "\n", encoding="utf-8")
print(json.dumps({"success": True, "output": str(args.output), "medians": medians, "comparison": comparison}))
