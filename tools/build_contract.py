"""Single source/policy inventory, MQB config generation and fail-closed drift checks."""
import argparse
import json
from pathlib import Path
import sys


def strict_pairs(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise ValueError(f"duplicate JSON key: {key}")
        result[key] = value
    return result


def read(path):
    return json.loads(path.read_text(encoding="utf-8-sig"), object_pairs_hook=strict_pairs)


def mqb_configuration(manifest):
    profiles = {}
    for config, policy in manifest["configurations"].items():
        # Static profiles must not inherit linker-only options.
        profiles[config] = {"build": {"configuration": config, "runtime": policy["runtime"],
            "defines": policy["defines"], "compiler_args": policy["compiler_args"]}}
        for name, target in manifest["targets"].items():
            if target["type"] != "exe":
                continue
            profiles[f"{name}-{config}"] = {"build": {**profiles[config]["build"],
                "subsystem": "console", "linker_args": policy["linker_args"], "libraries": manifest["msvc_system_libraries"] + target["libraries"]}}
    return {"version": 1, "build": {"entry": "tools/core_plan.cpp", "configuration": "debug",
        "architecture": manifest["architecture"], "standard": str(manifest["standard"]), "runtime": "MTd",
        "ltcg": False, "include_dirs": manifest["include_dirs"], "compiler_args": manifest["compiler_args"]},
        "discovery": {"enabled": True, "exclude_dirs": [".tools", "artifacts", "tests", "tasks", "docs", "viewer", "src/app", "src/d3d12"],
            "extra_sources": manifest["groups"]["core"]}, "profiles": profiles}


def validate(root, write=False):
    manifest = read(root / "build-manifest.json")
    if manifest["version"] != 1 or manifest["standard"] != 23 or manifest["architecture"] != "x64":
        raise ValueError("unsupported build manifest contract")
    declared = []
    for sources in manifest["groups"].values():
        declared.extend(sources)
    for name, target in manifest["targets"].items():
        declared.extend(target["sources"])
        if target["type"] not in ("exe", "static"):
            raise ValueError(f"unsupported target type: {name}")
        for group in target["groups"]:
            if group not in manifest["groups"]:
                raise ValueError(f"unknown source group: {group}")
    if len(declared) != len(set(declared)):
        raise ValueError("duplicate source declarations; use shared groups")
    for source in declared:
        p = Path(source)
        if p.is_absolute() or ".." in p.parts or not (root / p).is_file():
            raise ValueError(f"missing or out-of-project source: {source}")
    assets = [path for values in manifest.get("assets", {}).values() for path in values]
    if len(assets) != len(set(assets)):
        raise ValueError("duplicate asset declaration")
    for asset in assets:
        p = Path(asset)
        if p.is_absolute() or ".." in p.parts or not (root / p).is_file():
            raise ValueError(f"missing or out-of-project asset: {asset}")
    actual_assets = {p.relative_to(root).as_posix() for p in (root / "shaders").rglob("*") if p.is_file()}
    if actual_assets != set(assets):
        raise ValueError(f"asset inventory drift: unlisted={sorted(actual_assets-set(assets))}, stale={sorted(set(assets)-actual_assets)}")
    actual = {p.relative_to(root).as_posix() for directory in ("src", "tests", "tools") for p in (root / directory).rglob("*.cpp")}
    if actual != set(declared):
        raise ValueError(f"source inventory drift: unlisted={sorted(actual-set(declared))}, stale={sorted(set(declared)-actual)}")
    expected = mqb_configuration(manifest)
    config = root / "mqb.json"
    if write:
        config.write_text(json.dumps(expected, indent=2) + "\n", encoding="utf-8")
    if not config.is_file():
        raise ValueError("missing mqb.json; generate from the build manifest")
    if read(config) != expected:
        raise ValueError("MQB source/policy drift; review manifest and regenerate mqb.json")
    return manifest


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--write", action="store_true")
    args = parser.parse_args()
    try:
        manifest = validate(args.root.resolve(), args.write)
        print(f"build contract passed: {len(manifest['targets'])} targets; shared sources and MSVC policies")
    except (ValueError, KeyError, OSError) as error:
        print(f"build contract rejected: {error}", file=sys.stderr)
        sys.exit(1)
