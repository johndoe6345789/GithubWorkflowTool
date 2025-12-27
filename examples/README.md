# Example Workflows

This directory contains example GitHub Actions workflow files that can be used to test the GithubWorkflowTool.

## example-workflow.yml

A comprehensive example workflow that demonstrates:

1. **Multiple Triggers**:
   - Push events on main and develop branches
   - Pull request events
   - Manual workflow_dispatch

2. **Global Environment Variables**:
   - `BUILD_TYPE` set to Release

3. **Multiple Jobs with Dependencies**:
   - `build`: Initial build and test job
   - `test-matrix`: Matrix strategy testing (needs build)
   - `deploy`: Deployment job (needs both build and test-matrix)

4. **Matrix Strategy**:
   - Tests with multiple compilers (gcc, clang)
   - Tests with multiple build types (Debug, Release)

5. **Common Actions**:
   - `actions/checkout@v3`
   - `actions/upload-artifact@v3`

6. **Conditional Execution**:
   - Deploy job only runs on main branch

## Testing with GithubWorkflowTool

### Using CLI:
```bash
# Discover workflows in this directory
./build/gwt workflows examples/

# Run the example workflow
./build/gwt run examples/ examples/example-workflow.yml

# Run with QEMU backend
./build/gwt run examples/ examples/example-workflow.yml --qemu
```

### Using GUI:
1. Launch the GUI: `./build/gwt-gui`
2. Click "Clone Repository" (or use File > Open for local directory)
3. Select the examples directory
4. Select the workflow from the list
5. Choose backend (Container or QEMU)
6. Click "Run Workflow"
