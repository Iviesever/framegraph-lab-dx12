"""Negative source/policy drift tests using disposable fixtures inside artifacts."""
import importlib.util
import json
from pathlib import Path
import tempfile

root = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location("build_contract", root / "tools/build_contract.py")
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
scratch = root / "artifacts/build-contract-tests"
scratch.mkdir(parents=True, exist_ok=True)
assert scratch.resolve().is_relative_to(root.resolve())
with tempfile.TemporaryDirectory(dir=scratch) as directory:
    fixture = Path(directory)
    assert fixture.resolve().is_relative_to(root.resolve())
    manifest = module.read(root / "build-manifest.json")
    (fixture / "build-manifest.json").write_text(json.dumps(manifest), encoding="utf-8")
    for folder in ("src", "tests", "tools"):
        (fixture / folder).mkdir(exist_ok=True)
    paths = [p for group in manifest["groups"].values() for p in group]
    paths += [p for target in manifest["targets"].values() for p in target["sources"]]
    for path in paths:
        target = fixture / path
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text("// inventory fixture\n", encoding="utf-8")
    for path in [p for values in manifest.get("assets", {}).values() for p in values]:
        target = fixture / path
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text("// asset fixture\n", encoding="utf-8")
    module.validate(fixture, write=True)
    config = module.read(fixture / "mqb.json")
    config["profiles"]["release"]["build"]["runtime"] = "MD"
    (fixture / "mqb.json").write_text(json.dumps(config), encoding="utf-8")
    try:
        module.validate(fixture)
        raise AssertionError("runtime policy drift was accepted")
    except ValueError as error:
        assert "policy drift" in str(error)
    module.validate(fixture, write=True)
    (fixture / "src/unlisted.cpp").write_text("", encoding="utf-8")
    try:
        module.validate(fixture)
        raise AssertionError("unlisted C++ source was accepted")
    except ValueError as error:
        assert "inventory drift" in str(error)
    try:
        json.loads('{"version":1,"version":2}', object_pairs_hook=module.strict_pairs)
        raise AssertionError("duplicate key was accepted")
    except ValueError:
        pass
    (fixture / "src/unlisted.cpp").unlink()
    (fixture / manifest["assets"]["shaders"][0]).unlink()
    try:
        module.validate(fixture)
        raise AssertionError("missing shader asset was accepted")
    except ValueError as error:
        assert "asset" in str(error)
print("build contract negative tests passed: option drift, source drift, duplicate keys")
