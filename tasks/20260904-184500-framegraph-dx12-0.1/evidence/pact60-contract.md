# PACT-60 inspector/report acceptance before production changes

Consume the clean primary `artifacts/reports/framegraph-plan.json`, `frame-report.json` and `artifacts/captures/neon-ruins.png`. Generate `artifacts/viewer/framegraph-inspector.html` as one self-contained static file: no CDN, Node, runtime server, network fetch or hardcoded demo data.

RED validation must fail while the file is absent. GREEN validates embedded plan/report/image hashes, pass/resource/barrier counts, visible culled section, plan identity, memory ratio, all timing samples and no external URL. Then load the file in a real browser for desktop and narrow QA, inspect console, click pass/resource selections, and verify linked highlight changes.

The inspector must show pass DAG, stable order, dependency reasons, culled passes, resource lifetime bars, physical heap offsets/timeline, alias regions/chains, transition/UAV/alias barriers, GPU/CPU per-pass times, logical versus physical memory, identity/provenance and screenshot preview. Layout remains usable at 375px without horizontal page overflow. Use semantic HTML, accessible buttons/rows, keyboard focus and reduced-motion behavior.

Check in a small real sample plan/report/inspector and documentation screenshot. Large/full-resolution primary artifacts remain ignored. Checked samples state their renderer revision; final ignored delivery artifacts are regenerated from final clean HEAD.
