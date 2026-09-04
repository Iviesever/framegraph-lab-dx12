"""Verify embedded primary inputs and offline/static Inspector contract."""
import argparse
import base64
import hashlib
import json
from pathlib import Path
import re

parser = argparse.ArgumentParser()
parser.add_argument("--plan", type=Path, required=True)
parser.add_argument("--report", type=Path, required=True)
parser.add_argument("--image", type=Path, required=True)
parser.add_argument("--html", type=Path, required=True)
args = parser.parse_args()
html = args.html.read_text(encoding="utf-8")
assert len(html.encode("utf-8")) < 5_000_000
assert not re.search(r"(?:src|href)=[\"']https?://", html, re.I), "external dependency present"
for token in ("dag-panel", "lifetime-panel", "heap-panel", "barrier-panel", "timing-panel", "culled-panel", "memory-panel", "capture-preview", "selected-pass"):
    assert f'id="{token}"' in html, token
def embedded(name):
    match = re.search(rf"const {name} = '([A-Za-z0-9+/=]+)'", html)
    assert match, name
    return base64.b64decode(match.group(1))
plan_bytes = args.plan.read_bytes()
report_bytes = args.report.read_bytes()
image_bytes = args.image.read_bytes()
assert json.loads(embedded("PLAN_DATA")) == json.loads(plan_bytes)
assert json.loads(embedded("REPORT_DATA")) == json.loads(report_bytes)
assert embedded("IMAGE_DATA") == image_bytes
plan, report = json.loads(plan_bytes), json.loads(report_bytes)
assert plan["plan_identity"] == report["plan_identity"]
assert report["git_sha"] == plan["git_sha"] and report["source_clean"] and plan["source_clean"]
assert "addEventListener('click'" in html and "selectResource" in html and "selectPass" in html
assert "@media (max-width: 700px)" in html and "prefers-reduced-motion" in html
print(json.dumps({"success": True, "html_bytes": len(html.encode("utf-8")), "plan_identity": plan["plan_identity"],
    "embedded_png_sha256": hashlib.sha256(image_bytes).hexdigest(), "passes": len(plan["order"]), "resources": len(plan["resources"])}))
