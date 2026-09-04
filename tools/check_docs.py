"""Fail on missing required portfolio docs or broken repository-local Markdown links."""
from pathlib import Path
import re

root=Path(__file__).resolve().parents[1]
required=["README.md","docs/ARCHITECTURE.md","docs/RENDER_GRAPH_COMPILER.md","docs/DEPENDENCY_AND_CULLING.md","docs/TRANSIENT_MEMORY.md","docs/RESOURCE_BARRIERS.md","docs/D3D12_BACKEND.md","docs/CAPTURE_AND_REPORTS.md","docs/BENCHMARKING.md","docs/TESTING.md","docs/CODE_WALKTHROUGH.md","docs/INTERVIEW_GUIDE.md","docs/LIVE_CHANGE_DRILLS.md","docs/KNOWN_LIMITATIONS.md","docs/AI_ASSISTANCE.md","docs/RELEASE_NOTES_0.1_CANDIDATE.md"]
for item in required:
    assert (root/item).is_file(), item
for file in [root/"README.md",*(root/"docs").glob("*.md")]:
    text=file.read_text(encoding="utf-8")
    assert not re.search(r"\b(?:TBD|TODO|FIXME)\b",text),file
    for match in re.finditer(r"!?\[[^]]*\]\(([^)]+)\)",text):
        link=match.group(1).strip("<>").split("#",1)[0]
        if not link or re.match(r"(?:https?://|mailto:)",link): continue
        target=(root/link.lstrip("/")) if link.startswith("/") else (file.parent/link)
        assert target.resolve().is_relative_to(root.resolve()) and target.exists(),f"{file}: {link}"
print(f"documentation contract passed: {len(required)} required files and local links")
