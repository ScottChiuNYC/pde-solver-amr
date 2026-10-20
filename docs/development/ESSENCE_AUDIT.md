# Essence Audit

This journal records provenance only; it is not normative implementation authority.

## Status

- Essence: docs/development/essence.md
- Run: ICC-20261020-AMR — https://github.com/ScottChiuNYC/pde-solver-amr/issues/1
- Backend: GitHub
- State: FROZEN
- Skill revision: 9d1845f509a88dc8aad2f825f41a7009c3e77d6c
- Latest audited target: c02d302c5f0f945bcb3fd34a64fd10129ca0f0a5
- Latest round: R4
- Isolation: delivery_only; fresh contexts without inherited history and dispatcher-filtered verified inputs. Workers retain inherited tools; this is not an enforced access sandbox.

## Round R1

- Target commit: f6ea97425cb757d95b5cbb16832145144b6f3514
- Tree: 926f4d129bfda9338a911fcfddbd1f2e3032fc06
- Essence blob: bd1794968061c80cd49aee1bd165049817f58ff2
- Scope: complete current Draft, including preserved fixed-grid contract and additive adaptive spatial delta; Project Skeleton Contract v1.1.
- Control: https://github.com/ScottChiuNYC/pde-solver-amr/issues/2
- A1: FAIL — https://github.com/ScottChiuNYC/pde-solver-amr/issues/3
- A2: FAIL — https://github.com/ScottChiuNYC/pde-solver-amr/issues/4
- A3: FAIL — https://github.com/ScottChiuNYC/pde-solver-amr/issues/5
- Reconciliation: https://github.com/ScottChiuNYC/pde-solver-amr/issues/7
- Owner decisions: none established
- Round verdict: FAIL; next transition REMEDIATE

## Integration

The journal-only branch does not change audited normative authority. No solver implementation or new empirical acceptance obligation is introduced. The target repository has no CodeBinder/publication workflow; its normal build workflow is retained.

## R1 remediation

- Canonical findings: C-01 (midpoint representability transition), C-02 (Adaptive binary64 row schedule).
- Remediation record: https://github.com/ScottChiuNYC/pde-solver-amr/issues/8
- Repaired candidate Essence blob: e31493bd9b85469b8dde626e9eba426babcc7c62
- Authority: explicit Section 8.5 ICC clarification delegation; no Owner Decision.
- Static validation: only Sections 8.2–8.4 changed; Sections 1–7, 8.1 and 8.5 byte-identical. No API change, new empirical gate or solver implementation.
- Independent fresh exact-target audit remains required; remediation is not PASS.

## Round R2

- Target: 27b55391694ae1e59d09c04689e5385cad747777; Essence blob e31493bd9b85469b8dde626e9eba426babcc7c62.
- Control: https://github.com/ScottChiuNYC/pde-solver-amr/issues/9
- Fresh independent valid sealed results: A1 FAIL (#10), A2 FAIL (#11), A3 FAIL (#12).
- Reconciliation: https://github.com/ScottChiuNYC/pde-solver-amr/issues/13
- Round verdict: FAIL / REMEDIATE; new canonical C-03, later-refinement transfer arithmetic.
- C-01/C-02 did not recur in R2 findings; C-03 is not a repeated failed remediation.
- Owner decisions: none; existing Section 8.5 bounded clarification delegation applies.
- Remediation: https://github.com/ScottChiuNYC/pde-solver-amr/issues/14
- Only Section 8.4 transfer text changed; physical-coordinate weights at rounded midpoint and checked binary64 evaluation now specified. All other sections, prior midpoint/batch policy and Section 7 byte-identical.
- Candidate remains Draft; fresh full-scope exact-target audit required.
- Candidate 27b5539 baseline CI: Linux/Windows success, PR run 39472944336 and push run 39472937729. This is infrastructure evidence, not solver implementation acceptance.

## Round R3

- Target: f90372967c981f7e47b7a9b7fdde56e27c7c8a57; Essence blob d9177e725635f99b84e3ade3981a83baf6aef5bd.
- Control: https://github.com/ScottChiuNYC/pde-solver-amr/issues/15
- Fresh independent valid sealed results: A1 FAIL (#16), A2 FAIL (#17), A3 FAIL (#18).
- Reconciliation: https://github.com/ScottChiuNYC/pde-solver-amr/issues/19
- Round verdict: FAIL / REMEDIATE; new C-04, indicator arithmetic and nonfinite handling. Prior C-01/C-02/C-03 did not recur.
- Remediation: https://github.com/ScottChiuNYC/pde-solver-amr/issues/20
- Only Section 8.4 indicator text changed. Every other section and prior midpoint/transfer policy byte-identical; no new empirical gate, implementation or experiment.
- Owner decisions: none; existing Section 8.5 bounded delegation.
- Draft retained, fresh full-scope audit required.
- Candidate f903729 baseline CI: Linux/Windows success, PR run 39474003309 and push run 39473997805.

## Round R4 — PASS

- Target: c02d302c5f0f945bcb3fd34a64fd10129ca0f0a5; tree e3564695d50cc5616440f69a53bffb8f66e5ac5f; Essence blob 6d1f5b92dae8842a8d021fbbc3d27a70e15bd797.
- Control: https://github.com/ScottChiuNYC/pde-solver-amr/issues/21
- Fresh independent sealed valid results: A1 PASS (#22), A2 PASS (#23), A3 PASS (#24); zero findings, no declared forbidden exposure.
- Fresh reconciliation: https://github.com/ScottChiuNYC/pde-solver-amr/issues/25 — PASS / FREEZE.
- Canonical substantive findings: none. C-01 through C-04 closed by fresh full-scope audit; no unresolved Owner Decisions.
- Isolation: delivery_only; no inherited history, dispatcher-verified allowlisted packets, broader tools retained. This is not host-enforced access isolation.

## Freeze

- Reconciled PASS target: c02d302c5f0f945bcb3fd34a64fd10129ca0f0a5
- Frozen contract: docs/development/essence.md
- Frozen Essence blob: 7e2b3dff6feb858f5ad3d10667a202cdd137a486
- Frozen content SHA-256: 678e0d9227277c336887b290aa46c6e9a74880e60ec5798af81a42bb209fdb82
- Writeback is mechanical: title/status and non-normative PASS provenance only; inverse transformation reproduces the exact audited Essence.
- Final freeze commit and merged main identity: durable run record https://github.com/ScottChiuNYC/pde-solver-amr/issues/1 (recorded after commit/merge; no self-referential commit hash).
- Section 7 and all original Sections 1–7 domain semantics are unchanged; no solver implementation or new empirical acceptance promise.
- Final exact-head and merged-main integration evidence is recorded in run #1 / PR #6; generated skeleton workflows are unchanged. This repository has no CodeBinder/PDF publication workflow.
