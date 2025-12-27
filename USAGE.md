# GithubWorkflowTool User Guide

## Overview

GithubWorkflowTool is a desktop application that allows you to clone Git repositories and execute their GitHub Actions workflows locally on your machine. This is useful for:

- Testing workflows before pushing to GitHub
- Debugging workflow issues offline
- Understanding how workflows behave in different environments
- Rapid iteration on workflow changes

## Installation

See [BUILD.md](BUILD.md) for build and installation instructions.

## Core Concepts

### Repository Storage

GithubWorkflowTool clones repositories to platform-specific cache directories:

- **Windows**: `%APPDATA%\GithubWorkflowTool\repos\`
- **Linux**: `$XDG_DATA_HOME/githubworkflowtool/repos/` (or `~/.local/share/githubworkflowtool/repos/`)

Each repository is stored with a unique key based on its URL, preventing conflicts.

### Execution Backends

Two backends are available for running workflows:

1. **Container Backend** (Default)
   - Uses Docker or Podman
   - Fast iteration
   - Lower resource usage
   - Automatically maps GitHub runner specs to container images

2. **QEMU Backend**
   - Uses QEMU virtual machines
   - Higher fidelity to actual GitHub runners
   - Requires VM images (not included)
   - Better isolation

## Command-Line Interface (CLI)

### Basic Commands

#### Clone a Repository
```bash
gwt clone https://github.com/owner/repo
```

Clone a specific branch:
```bash
gwt clone https://github.com/owner/repo --branch develop
```

#### List Cloned Repositories
```bash
gwt list
```

#### Discover Workflows
```bash
gwt workflows /path/to/repo
```

#### Check System and Workflow Compatibility
```bash
# Check system backends (Docker, Podman, QEMU)
gwt doctor

# Check specific workflow for compatibility issues
gwt doctor /path/to/repo/.github/workflows/ci.yml
```

The `doctor` command diagnoses:
- Backend availability (Docker, Podman, QEMU)
- Workflow parsing errors
- Unsupported features (service containers, reusable workflows, etc.)
- macOS runner usage
- Advanced expression usage
- Job dependency issues

**Always run `gwt doctor` before executing workflows to identify potential issues early.**

#### Run a Workflow
```bash
gwt run /path/to/repo /path/to/repo/.github/workflows/ci.yml
```

Run with QEMU backend:
```bash
gwt run /path/to/repo /path/to/repo/.github/workflows/ci.yml --qemu
```

### Advanced Usage

#### Triggering Specific Events
By default, workflows are triggered with a "push" event. You can specify different trigger events:

```bash
gwt run /path/to/repo /path/to/workflow.yml --event pull_request
```

#### Environment Variables
Pass environment variables to the workflow:

```bash
gwt run /path/to/repo /path/to/workflow.yml --env BUILD_TYPE=Release --env DEBUG=1
```

## Graphical User Interface (GUI)

Launch the GUI:
```bash
gwt-gui
```

### GUI Workflow

1. **Clone or Open Repository**
   - Click "Clone Repository" to clone from a URL
   - Or use File > Open to browse for a local repository

2. **Select Repository**
   - View all cloned repositories in the left panel
   - Click a repository to load its workflows

3. **View Workflows**
   - Available workflows appear in the middle panel
   - Each workflow shows its name and trigger events

4. **Configure Execution**
   - Select a workflow
   - Choose backend (Container or QEMU)
   - Set any environment variables

5. **Run and Monitor**
   - Click "Run Workflow"
   - View real-time output in the bottom panel
   - See job progress and status updates

## Workflow Feature Support

### Supported Features

✅ **Basic Workflow Structure**
- Jobs and steps
- Job dependencies (needs)
- Environment variables (global and per-job)
- Working directory
- Shell selection

✅ **Triggers**
- push
- pull_request
- workflow_dispatch
- schedule (basic support)

✅ **Matrix Strategies**
- strategy.matrix expansion
- Multiple dimensions
- Include/exclude (partial)

✅ **Artifacts**
- Upload artifacts (stored locally)
- Download artifacts between jobs
- Artifact retention

✅ **Caching**
- Cache save and restore
- Key-based caching
- Path patterns

✅ **Common Actions** (Container mode)
- actions/checkout@v3
- actions/upload-artifact@v3
- actions/download-artifact@v3
- actions/cache@v3

### Limited Support

⚠️ **Conditional Execution**
- Basic `if` conditions
- Context variables (partial)

⚠️ **Third-Party Actions**
- Some actions work in container mode
- Others may require manual setup

### Not Supported (v1)

❌ Service containers
❌ Secrets management
❌ OIDC authentication
❌ Self-hosted runner features
❌ macOS runner images

## Examples

### Example 1: Simple CI Workflow

Create `.github/workflows/ci.yml`:

```yaml
name: CI

on:
  push:
    branches: [ main ]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Build
        run: make build
      - name: Test
        run: make test
```

Run it:
```bash
gwt clone https://github.com/user/repo
gwt run ~/.local/share/githubworkflowtool/repos/github.com/user/repo_* ~/.local/share/githubworkflowtool/repos/github.com/user/repo_*/.github/workflows/ci.yml
```

### Example 2: Matrix Strategy

```yaml
name: Test Matrix

on: push

jobs:
  test:
    runs-on: ubuntu-latest
    strategy:
      matrix:
        python: ['3.8', '3.9', '3.10']
        os: [ubuntu-latest, windows-latest]
    steps:
      - name: Test Python ${{ matrix.python }} on ${{ matrix.os }}
        run: python --version
```

This expands to 6 jobs (3 Python versions × 2 OS types).

### Example 3: Dependent Jobs

```yaml
name: Deploy

on: push

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - name: Build
        run: echo "Building..."
  
  test:
    needs: build
    runs-on: ubuntu-latest
    steps:
      - name: Test
        run: echo "Testing..."
  
  deploy:
    needs: [build, test]
    runs-on: ubuntu-latest
    steps:
      - name: Deploy
        run: echo "Deploying..."
```

Jobs run in order: build → test → deploy

## Troubleshooting

### Quick Diagnostics

**Before troubleshooting, always run:**
```bash
gwt doctor /path/to/workflow.yml
```

This will identify most common issues and suggest workarounds.

### Container Backend Issues

**Problem**: "Docker not found"
**Solution**: Install Docker or Podman

**Problem**: "Permission denied accessing Docker"
**Solution**: Add user to docker group: `sudo usermod -aG docker $USER`

**Problem**: "Cannot pull image"
**Solution**: Check internet connection or use `docker pull ubuntu:22.04` manually

### QEMU Backend Issues

**Problem**: "QEMU not found"
**Solution**: Install QEMU: `sudo apt install qemu-system-x86`

**Problem**: "VM image not found"
**Solution**: QEMU backend requires pre-built VM images. See documentation for creating images.

### Workflow Issues

**Problem**: "Workflow parsing errors"
**Solution**: Validate YAML syntax using `yamllint` or GitHub's workflow validator

**Problem**: "Action not found"
**Solution**: Some actions may not work locally. Try using `run` steps instead.

**Problem**: "Step timeout"
**Solution**: Increase timeout in step definition or check for hanging processes

## Tips and Best Practices

1. **Use Container Backend for Development**
   - Faster startup and iteration
   - Easier debugging

2. **Use QEMU Backend for Final Testing**
   - Higher fidelity to GitHub runners
   - Better for compatibility testing

3. **Test Matrix Strategies Locally**
   - Catch issues before pushing to GitHub
   - Save CI minutes

4. **Cache Dependencies**
   - Use actions/cache or similar
   - Speeds up repeated runs

5. **Keep Workflows Simple**
   - Complex workflows may have compatibility issues
   - Break into smaller jobs for easier debugging

## Further Reading

- [GitHub Actions Documentation](https://docs.github.com/en/actions)
- [Workflow Syntax Reference](https://docs.github.com/en/actions/reference/workflow-syntax-for-github-actions)
- [BUILD.md](BUILD.md) - Build instructions
- [examples/](examples/) - Example workflows
