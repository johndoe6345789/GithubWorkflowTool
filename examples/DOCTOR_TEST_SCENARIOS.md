# Test Scenario: gwt doctor command

## Purpose
This document demonstrates the expected behavior of the `gwt doctor` command implementation for various scenarios.

## Test Case 1: System Diagnostics Only (No Workflow File)

### Command:
```bash
gwt doctor
```

### Expected Output:
```
GithubWorkflowTool Diagnostics
==============================

Backend Availability:
✓ Container backend: Docker detected (24.0.0)
✓ QEMU backend: Available (7.2.0)

Summary: No issues detected
✓ System is ready for workflow execution

For detailed information on limitations and workarounds, see LIMITATIONS.md
```

**Exit Code:** 0

---

## Test Case 2: System with Missing QEMU

### Command:
```bash
gwt doctor
```

### Expected Output:
```
GithubWorkflowTool Diagnostics
==============================

Backend Availability:
✓ Container backend: Docker detected (24.0.0)
⚠ QEMU backend: Not available
  → Install QEMU for VM-based execution (optional)

Summary: 1 issue(s) detected (1 warning(s))
⚠ Workflow should run with noted limitations

For detailed information on limitations and workarounds, see LIMITATIONS.md
```

**Exit Code:** 0 (warnings don't cause failure)

---

## Test Case 3: System with No Container Runtime

### Command:
```bash
gwt doctor
```

### Expected Output:
```
GithubWorkflowTool Diagnostics
==============================

Backend Availability:
✗ Container backend: Neither Docker nor Podman found
  → Install Docker or Podman for container backend support
✓ QEMU backend: Available (7.2.0)

Summary: 1 issue(s) detected (1 error(s))
⚠ Workflow may fail or require modifications to run successfully

For detailed information on limitations and workarounds, see LIMITATIONS.md
```

**Exit Code:** 1 (errors cause failure)

---

## Test Case 4: Valid Workflow File

### Command:
```bash
gwt doctor examples/example-workflow.yml
```

### Expected Output:
```
GithubWorkflowTool Diagnostics
==============================

Backend Availability:
✓ Container backend: Docker detected (24.0.0)
✓ QEMU backend: Available (7.2.0)

Workflow: example-workflow.yml
✓ Basic workflow structure valid
✓ Job dependencies resolvable

Summary: No issues detected
✓ System is ready for workflow execution

For detailed information on limitations and workarounds, see LIMITATIONS.md
```

**Exit Code:** 0

---

## Test Case 5: Workflow with Limitations

### Command:
```bash
gwt doctor examples/workflow-with-limitations.yml
```

### Expected Output:
```
GithubWorkflowTool Diagnostics
==============================

Backend Availability:
✓ Container backend: Docker detected (24.0.0)
✓ QEMU backend: Available (7.2.0)

Workflow: workflow-with-limitations.yml
✓ Basic workflow structure valid
⚠ Warning: Service containers detected (not supported in v1)
  → Workaround: Run services manually before workflow execution
  → See LIMITATIONS.md §3.3
⚠ Warning: Uses 'hashFiles' expression (limited support in v1)
  → Workaround: Simplify expression or use explicit run steps
  → See LIMITATIONS.md §3.1
✗ Error: macOS runners not supported in v1
  → Workaround: Use Linux runners or run macOS workflows on GitHub
  → See LIMITATIONS.md §3.4
⚠ Warning: Concurrency groups (not supported in v1)
  → Note: All jobs run as configured, no concurrency limits applied
  → See LIMITATIONS.md §3.1

Summary: 4 issue(s) detected (3 warning(s), 1 error(s))
⚠ Workflow may fail or require modifications to run successfully

For detailed information on limitations and workarounds, see LIMITATIONS.md
```

**Exit Code:** 1 (errors present)

---

## Test Case 6: Workflow with Invalid Dependencies

### Command:
```bash
gwt doctor test-workflow-bad-deps.yml
```

### Test Workflow (test-workflow-bad-deps.yml):
```yaml
name: Bad Dependencies
on: push
jobs:
  test:
    runs-on: ubuntu-latest
    needs: [build]  # 'build' job doesn't exist!
    steps:
      - run: echo "test"
```

### Expected Output:
```
GithubWorkflowTool Diagnostics
==============================

Backend Availability:
✓ Container backend: Docker detected (24.0.0)
✓ QEMU backend: Available (7.2.0)

Workflow: test-workflow-bad-deps.yml
✓ Basic workflow structure valid
✗ Error: Job 'test' depends on non-existent job 'build'

Summary: 1 issue(s) detected (1 error(s))
⚠ Workflow may fail or require modifications to run successfully

For detailed information on limitations and workarounds, see LIMITATIONS.md
```

**Exit Code:** 1

---

## Test Case 7: Workflow File Not Found

### Command:
```bash
gwt doctor nonexistent.yml
```

### Expected Output:
```
GithubWorkflowTool Diagnostics
==============================

Backend Availability:
✓ Container backend: Docker detected (24.0.0)
✓ QEMU backend: Available (7.2.0)

Error: Workflow file not found: nonexistent.yml
```

**Exit Code:** 1

---

## Test Case 8: Workflow with Parse Errors

### Command:
```bash
gwt doctor invalid-syntax.yml
```

### Test Workflow (invalid-syntax.yml):
```yaml
name: Invalid Syntax
on push  # Missing colon - syntax error!
jobs:
  test:
    runs-on: ubuntu-latest
```

### Expected Output:
```
GithubWorkflowTool Diagnostics
==============================

Backend Availability:
✓ Container backend: Docker detected (24.0.0)
✓ QEMU backend: Available (7.2.0)

Workflow: invalid-syntax.yml
✗ Error: Workflow parsing failed
  YAML parse error at line 2: ...

Summary: 1 issue(s) detected (1 error(s))
⚠ Workflow may fail or require modifications to run successfully

For detailed information on limitations and workarounds, see LIMITATIONS.md
```

**Exit Code:** 1

---

## Implementation Notes

### Detection Logic

1. **Backend Detection:**
   - Try Docker first with `docker --version`
   - Fall back to Podman with `podman --version`
   - Check QEMU with `qemu-system-x86_64 --version`
   - Timeouts: 3 seconds per check

2. **Workflow Analysis:**
   - Parse with WorkflowParser
   - Check job dependencies with QMap::contains()
   - String search for limitation patterns:
     - `workflow_call` → reusable workflows
     - `services:` → service containers
     - `fromJSON`, `hashFiles`, `toJSON` → advanced expressions
     - `macos-latest`, `macos-` → macOS runners
     - `concurrency:` → concurrency groups

3. **Exit Codes:**
   - 0: Success or warnings only
   - 1: Errors detected or file not found

### Code Quality
- Uses modern C++ range-based loops with structured bindings
- Proper Qt resource management (QFile, QProcess)
- Clear separation of concerns
- Actionable error messages with workarounds

---

## Integration with Development Workflow

### Recommended Usage

**Before first run:**
```bash
gwt doctor
```

**Before running a specific workflow:**
```bash
gwt doctor path/to/workflow.yml
```

**In CI pipelines:**
```bash
# Check compatibility before attempting execution
if ! gwt doctor .github/workflows/ci.yml; then
    echo "Workflow has compatibility issues, see output above"
    exit 1
fi
```

### Benefits
1. **Fail Fast:** Catch issues before workflow execution
2. **Clear Guidance:** Actionable workarounds for each limitation
3. **Version Awareness:** Links to LIMITATIONS.md with resolution roadmap
4. **Progressive Enhancement:** Warnings vs errors guide user decisions
