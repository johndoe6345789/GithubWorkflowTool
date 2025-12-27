# Known Limitations & Resolution Strategy (v1)

## 1. Scope

This document formally defines acknowledged limitations of GithubWorkflowTool v1, explains why they exist, and documents the intended resolution path for future versions.

The goal is **transparency, predictability, and a clear upgrade trajectory** rather than silent incompatibility.

---

## 2. Limitation Categories

Limitations are grouped by capability class and assigned a resolution tier:

- **Deferred** – intentionally excluded from v1, planned for v2+
- **Constrained** – partially supported with explicit restrictions
- **External** – blocked by legal, platform, or ecosystem constraints
- **Configurable** – disabled or limited by default but unlockable

---

## 3. Detailed Limitations

### 3.1 Incomplete GitHub Actions Feature Coverage

**Status**
- Type: **Deferred**
- Severity: **Medium**
- Impact: Some workflows may fail or require modification

**Description**

GithubWorkflowTool v1 implements a subset of GitHub Actions semantics. Unsupported or partially supported features include (non-exhaustive):

- Advanced expressions (`fromJSON`, `hashFiles`, complex boolean chains)
- Reusable workflows (`workflow_call`)
- Dynamic job generation outside `strategy.matrix`
- Job-level permissions and token scoping
- Environments with approval gates
- Concurrency groups

**Rationale**

GitHub Actions is not formally specified; it is an evolving behavior-driven platform. v1 prioritizes **deterministic local execution** over speculative compatibility.

**Resolution Path**
- **v2**: Expand expression engine coverage
- **v2**: Add reusable workflow resolution
- **v3**: Permissions & environment gates (read-only simulation)

**User Guidance (v1)**
- Prefer explicit `run:` steps over deeply nested expressions
- Avoid reusable workflows unless flattened
- Use matrix strategies only for static axes

---

### 3.2 Third-Party Actions Compatibility

**Status**
- Type: **Constrained**
- Severity: **Medium–High**
- Impact: Some marketplace actions may fail

**Description**

Some third-party actions assume:
- Preinstalled tools on GitHub-hosted runners
- Full Node.js runtime availability
- Docker-in-Docker capability
- Privileged filesystem or network access

v1 does not guarantee these assumptions unless explicitly provided by the selected runner backend.

**Supported Action Types (v1)**
- Local composite actions
- Docker-based actions
- Node-based actions **only if**:
  - Container backend is used, **or**
  - Required runtimes are present in the VM template

**Resolution Path**
- **v2**: Action capability probing (pre-flight validation)
- **v2**: Curated "known-good" action compatibility list
- **v3**: Optional toolchain auto-provisioning

**User Guidance (v1)**
- Prefer Docker-based actions
- Pin action versions explicitly
- Use container backend when testing marketplace actions

---

### 3.3 Service Containers Not Implemented

**Status**
- Type: **Deferred**
- Severity: **High** (for integration-heavy workflows)
- Impact: DB-backed or multi-service workflows will fail

**Description**

GitHub Actions `services:` (e.g., PostgreSQL, Redis) are not supported in v1.

There is no orchestration layer for:
- Service lifecycle management
- Network aliasing
- Health checks
- Port mapping into jobs

**Rationale**

Service containers introduce orchestration complexity comparable to a lightweight Kubernetes layer. This is intentionally excluded from v1 to keep execution semantics simple and debuggable.

**Resolution Path**
- **v2**: Container-backend-only service support
- **v2.1**: Declarative service lifecycle
- **v3**: QEMU backend service networking

**User Guidance (v1)**

Replace services with:
- External services (run manually, point workflows at fixed endpoints)
- Embedded test doubles
- Single-container workflows
- Manual database/service setup before workflow execution

**Example Workaround:**

Instead of:
```yaml
jobs:
  test:
    services:
      postgres:
        image: postgres:14
```

Use:
```bash
# Run service manually
docker run -d -p 5432:5432 -e POSTGRES_PASSWORD=test postgres:14

# Then run workflow
gwt run /path/to/repo /path/to/workflow.yml --env DATABASE_URL=postgresql://localhost:5432
```

---

### 3.4 macOS Runner Images Unsupported

**Status**
- Type: **External**
- Severity: **High** (for Apple-specific projects)
- Impact: macOS workflows cannot be executed

**Description**

macOS GitHub-hosted runners are not supported in v1.

**Reasons:**
- Apple licensing restrictions
- Virtualization limitations outside Apple hardware
- Lack of redistributable macOS base images

**Resolution Path**
- **v2**: "macOS-host-like" profile (best-effort simulation on Linux)
- **v3**: Native macOS-host execution (host-only, no VM)
- **No QEMU-based macOS support planned** (licensing constraints)

**User Guidance (v1)**
- Use Linux runners for cross-platform builds where possible
- Split workflows:
  - Linux CI locally (with GithubWorkflowTool)
  - macOS CI only on GitHub
- For macOS-specific testing: use GitHub Actions cloud runners

---

### 3.5 Limited Network Access in Runners

**Status**
- Type: **Configurable**
- Severity: **Medium**
- Impact: Actions requiring external downloads may fail

**Description**

Network access inside runners may be:
- Restricted
- Rate-limited
- Disabled entirely (offline mode)

This applies especially to:
- QEMU backend
- Reproducibility-focused runs
- CI debugging scenarios

**Rationale**

Unrestricted network access:
- Breaks reproducibility
- Introduces nondeterministic failures
- Can leak secrets unintentionally

**Resolution Path**
- **v1.1**: Domain allowlist support
- **v2**: Network profiles per runner
- **v2**: Cached dependency mirrors
- **v3**: Deterministic fetch recording & replay

**User Guidance (v1)**
- Vendor dependencies where possible
- Use local mirrors or caches
- Enable network explicitly when required (container mode allows network by default)
- For offline testing, pre-download all dependencies

---

## 4. Limitation Visibility & UX Requirements

### CLI Requirements

The `gwt doctor` command must report:
- Unsupported workflow features detected in a workflow file
- Action incompatibilities
- Missing toolchains (Docker, Podman, QEMU)
- Runner backend availability
- Clear warnings before execution, not after failure

**Example Output:**
```bash
$ gwt doctor /path/to/repo/.github/workflows/ci.yml

GithubWorkflowTool Diagnostics
==============================

Workflow: ci.yml
✓ Basic workflow structure valid
✓ Job dependencies resolvable
⚠ Warning: Uses reusable workflow (not supported in v1)
⚠ Warning: Service container 'postgres' detected (not supported in v1)
  → Workaround: Run PostgreSQL manually before workflow execution
✗ Error: Uses 'hashFiles' expression (not supported in v1)
  → Resolution: Simplify expression or upgrade to v2 (planned)

Backend Availability:
✓ Container backend: Docker detected (v24.0.0)
✓ QEMU backend: Available (v7.2.0)

Recommendation: 3 issues detected, 2 warnings, 1 error
Workflow may fail or require modifications to run successfully.
```

### GUI Requirements

Visual badges on workflows/jobs:
- "✓ Fully Supported"
- "⚠ Partially Supported" (with details in hover/tooltip)
- "✗ Unsupported in v1" (with workaround in detail pane)
- "Requires Container Backend"
- "Requires QEMU Backend"

When hovering over a badge or clicking for details, explain:
- What limitation applies
- Why it exists
- Recommended workaround
- Expected version for full support

---

## 5. Versioning & Compatibility Policy

### v1.x
- No breaking behavior changes
- Limitations remain explicit and documented
- New features may be added if backward compatible
- Unsupported features fail fast with clear error messages

### v2.0
- Feature expansion allowed
- Backward compatibility preferred but not guaranteed
- Migration guide provided for breaking changes
- Unsupported features must fail fast, not silently degrade

### Future Versions
- Incremental improvements to limitation resolution
- Regular compatibility matrix updates
- Community feedback integration

---

## 6. Acceptance Criteria (Limitations Spec)

This specification is satisfied when:

1. ✅ All listed limitations are explicitly detected or documented at runtime
2. ✅ Failures caused by limitations produce actionable diagnostics
3. ✅ Users can predict before execution whether a workflow is likely to succeed
4. ✅ Resolution paths are versioned and traceable
5. ✅ Documentation is accessible and linked from main README

---

## 7. Compatibility Matrix (Preview)

A detailed compatibility matrix mapping common GitHub Actions to backend support will be provided in a future specification. Preview:

| Action | Container | QEMU | Status | Notes |
|--------|-----------|------|--------|-------|
| actions/checkout@v3 | ✓ | ✓ | Full | Git must be available |
| actions/cache@v3 | ⚠ | ⚠ | Partial | Local cache only |
| actions/setup-node@v3 | ✓ | ✓ | Full | Node must be in image |
| docker/build-push-action@v4 | ✓ | ✗ | Container only | DinD required |
| Service containers | ✗ | ✗ | Not supported | v2 planned |
| Reusable workflows | ✗ | ✗ | Not supported | v2 planned |

Full matrix to be documented in `COMPATIBILITY_MATRIX.md` (future work).

---

## 8. Feedback and Contributions

Users encountering limitations not documented here should:
1. Check for existing GitHub issues
2. Report new limitations with:
   - Workflow file (or minimal example)
   - Error message
   - Expected behavior
   - Actual behavior
3. Contribute workarounds to documentation

---

## Summary

GithubWorkflowTool v1 is designed with **intentional constraints** to ensure:
- Predictable, deterministic execution
- Clear error messages and diagnostics
- Transparent limitations with documented workarounds
- Traceable resolution path for future versions

Users should consult this document and use the `gwt doctor` command before deploying workflows to understand compatibility and required modifications.

For questions or issues, please refer to the GitHub repository issue tracker.
