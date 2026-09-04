"""Generate process-local build provenance without touching tracked source files."""
from pathlib import Path
import re
import subprocess

root = Path(__file__).resolve().parents[1]
try:
    sha = subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=root, stderr=subprocess.DEVNULL, timeout=10).decode().strip()
    clean = not subprocess.check_output(["git", "status", "--porcelain"], cwd=root, stderr=subprocess.DEVNULL, timeout=10).strip()
except (OSError, subprocess.SubprocessError):
    sha, clean = "unknown", False
if sha != "unknown" and not re.fullmatch(r"[0-9a-f]{40}", sha):
    raise ValueError("invalid Git revision")
path = root / "artifacts/generated/revision.hpp"
path.parent.mkdir(parents=True, exist_ok=True)
text = f'#pragma once\nnamespace fgl::build_revision {{ inline constexpr char sha[] = "{sha}"; inline constexpr bool clean = {str(clean).lower()}; }}\n'
if not path.exists() or path.read_text(encoding="utf-8") != text:
    path.write_text(text, encoding="utf-8")
