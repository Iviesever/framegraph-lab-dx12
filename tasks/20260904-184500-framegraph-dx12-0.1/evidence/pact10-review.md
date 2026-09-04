# Preliminary independent compiler review

Fresh read-only agent inspected uncommitted PACT-10 against origin/main fb23f40818661ed791854181f33ee22bf6ee57fc. No confirmed Blocker/High in dependencies/topology/cycle/culling/bounds. No source changes or GPU runs by reviewer.

- Confirmed Medium: UTF-8 bytes became Latin-1 escaped code points. Regression `valid_utf8_names_round_trip` failed, then passed after preserving valid UTF-8 and escaping malformed bytes.
- Export lifetime ambiguity: output could be reused before graph completion. Contract resolved in favor of preserving exported contents to completion; `export_contents_survive_graph_completion` failed then passed. Property lifetime oracle updated independently.
- Suggested remaining tests: exact usage/edge bounds, invalid format/import states, imported WAR and RW chains, overwrite retention, diagnostic edge membership, unused imported final-state barrier.

Final full-project audit remains pending and must cover D3D12 and delivery evidence too.
