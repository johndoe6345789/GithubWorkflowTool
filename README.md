# GithubWorkflowTool

A cross-platform desktop application that clones Git repositories to per-user cache directories and simulates GitHub Actions workflows locally.

## Features

- **Repository Management**: Clone repositories to platform-specific cache directories
  - Windows: `%APPDATA%\GithubWorkflowTool\repos\`
  - Linux: `$XDG_DATA_HOME/githubworkflowtool/repos/` or `~/.local/share/githubworkflowtool/repos/`

- **Workflow Simulation**: Discover and execute GitHub Actions workflows locally
  - Automatic discovery of `.github/workflows/*.yml` files
  - Support for jobs, steps, needs, env variables, and outputs
  - Matrix strategy expansion
  - Artifacts upload/download (local equivalent)
  - Caching support (local equivalent of actions/cache)

- **Multiple Execution Backends**:
  - **Container Backend**: Fast iteration using Docker or Podman
  - **QEMU Backend**: Higher fidelity VM-based execution

- **Dual Interface**:
  - **CLI**: Command-line interface for automation and scripting
  - **GUI**: Qt6-based graphical interface for visual inspection and interaction

## Technology Stack

- C++17
- Qt 6.6+
- CMake 3.22+
- Conan package manager
- Ninja build system
- yaml-cpp for workflow parsing
- Docker/Podman (optional, for container backend)
- QEMU (optional, for VM backend)

## Quick Start

See [BUILD.md](BUILD.md) for detailed build instructions.

```bash
# Install dependencies
conan install . --output-folder=build --build=missing

# Build
cmake --preset=default
cmake --build build

# Run CLI
./build/gwt clone https://github.com/user/repo
./build/gwt workflows /path/to/repo
./build/gwt run /path/to/repo /path/to/workflow.yml

# Run GUI
./build/gwt-gui
```

## Architecture

### Core Components

- **StorageProvider**: Platform-specific storage path management
- **RepoManager**: Git repository cloning and management
- **WorkflowDiscovery**: Workflow file discovery
- **WorkflowParser**: YAML workflow parsing
- **JobExecutor**: Workflow execution orchestration
- **MatrixStrategy**: Matrix strategy expansion
- **ArtifactManager**: Artifact upload/download
- **CacheManager**: Cache management

### Backends

- **ContainerBackend**: Executes workflows in Docker/Podman containers
- **QemuBackend**: Executes workflows in QEMU virtual machines

## Repository Structure

```
GithubWorkflowTool/
├── CMakeLists.txt          # Main CMake configuration
├── conanfile.txt           # Conan dependencies
├── CMakePresets.json       # CMake presets
├── include/                # Public headers
│   ├── core/              # Core functionality headers
│   ├── backends/          # Execution backend headers
│   ├── cli/               # CLI headers
│   └── gui/               # GUI headers
├── src/                    # Implementation files
│   ├── core/              # Core functionality
│   ├── backends/          # Execution backends
│   ├── cli/               # CLI implementation
│   └── gui/               # GUI implementation
├── BUILD.md               # Build instructions
└── README.md              # This file
```

## Supported Platforms

- Windows 11+
- Linux (Ubuntu/Debian-based distributions)
- macOS (host support; VM images may have limitations)

## Limitations (v1)

- Not all GitHub Actions features are supported
- Some third-party actions may require container mode or preinstalled tools
- Service containers not yet implemented
- macOS runner images not supported
- Network access in runners may be limited

## License

See [LICENSE](LICENSE) file for details.

## Contributing

Contributions are welcome! Please ensure your code follows the existing style and includes appropriate tests.
