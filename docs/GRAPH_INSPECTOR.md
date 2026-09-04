# Offline graph inspector

`tools/build_inspector.py` consumes canonical plan JSON, runtime report JSON and the matching PNG. It rejects plan/report identity or Git provenance mismatch and dirty inputs. The output is one UTF-8 HTML file containing base64 copies of every input; no CDN, fetch, Node runtime or server is required after generation.

The header exposes schema, scene seed, pass/resource/edge counts and exact identity/SHA. The DAG draws real dependency edges behind ordered pass buttons. Resource lifetime rows use inclusive plan positions; physical heap bars use returned offsets/sizes and mark shared regions. Alias text retains predecessor/successor. Barrier rows list phase, kind, resource and states. Timing bars use real report samples and memory bars separate logical/reference/actual values. Culled data is visible even when the set is empty.

Selecting a pass highlights its declared resources and filters barriers/timing. Selecting a resource highlights every consuming pass and relevant barriers, with heap/offset/lifetime detail. Buttons have keyboard focus and Escape returns to the first pass. Reduced-motion style removes transitions.

Primary generation and static verification:

```powershell
python tools/build_inspector.py --plan artifacts/reports/framegraph-plan.json --report artifacts/reports/frame-report.json --image artifacts/captures/neon-ruins.png --output artifacts/viewer/framegraph-inspector.html
python tests/viewer/validate_inspector.py --plan artifacts/reports/framegraph-plan.json --report artifacts/reports/frame-report.json --image artifacts/captures/neon-ruins.png --html artifacts/viewer/framegraph-inspector.html
```

Browser QA used the Codex in-app browser against a temporary localhost server. At 1280px: body 1264.7px, page scroll width 1265px, 8 pass nodes and 7 resource rows. Console Error/Warning was empty. SceneHDR selection showed Depth Read/DepthRead and SceneHDR Write/RenderTarget; BloomC selection highlighted two relevant passes and its offset-zero reuse. At 375px, the first version allowed 52px page horizontal scroll. After RED→GREEN CSS change, document client/scroll width were both 360px and horizontal scroll reached 0; the image was 306.7px wide. Internal DAG/table areas retain independent scrolling. The temporary viewport was reset, browser tab closed and owned HTTP server stopped.

`viewer/` contains a checked-in real sample generated from the PACT-50 renderer revision recorded in `SAMPLE_PROVENANCE.md`. Ignored delivery artifacts are regenerated from the final clean HEAD during PACT-80.
