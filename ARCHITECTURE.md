# GithubWorkflowTool Architecture

## Overview

GithubWorkflowTool is a C++/Qt6 desktop application that simulates GitHub Actions workflows locally. The architecture is modular, with clear separation between core functionality, execution backends, and user interfaces.

## System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     User Interfaces                          │
│  ┌──────────────────────┐    ┌─────────────────────────┐   │
│  │    CLI (gwt)         │    │   GUI (gwt-gui)         │   │
│  │  - CommandHandler    │    │   - MainWindow          │   │
│  │  - main.cpp          │    │   - WorkflowView        │   │
│  │                      │    │   - JobView             │   │
│  └──────────┬───────────┘    └───────────┬─────────────┘   │
└─────────────┼────────────────────────────┼─────────────────┘
              │                            │
              └────────────┬───────────────┘
                           ▼
┌─────────────────────────────────────────────────────────────┐
│                      Core Layer                              │
│  ┌─────────────────────────────────────────────────────┐   │
│  │  Repository Management                               │   │
│  │  - StorageProvider (platform-specific paths)         │   │
│  │  - RepoManager (Git operations)                      │   │
│  │  - WorkflowDiscovery (.github/workflows finder)      │   │
│  └─────────────────────────────────────────────────────┘   │
│  ┌─────────────────────────────────────────────────────┐   │
│  │  Workflow Processing                                 │   │
│  │  - WorkflowParser (YAML parsing)                     │   │
│  │  - MatrixStrategy (matrix expansion)                 │   │
│  │  - JobExecutor (orchestration)                       │   │
│  └─────────────────────────────────────────────────────┘   │
│  ┌─────────────────────────────────────────────────────┐   │
│  │  Resource Management                                 │   │
│  │  - ArtifactManager (local artifacts)                 │   │
│  │  - CacheManager (local caching)                      │   │
│  └─────────────────────────────────────────────────────┘   │
└──────────────────────────┬──────────────────────────────────┘
                           ▼
┌─────────────────────────────────────────────────────────────┐
│                   Execution Backends                         │
│  ┌──────────────────────┐    ┌─────────────────────────┐   │
│  │  ContainerBackend    │    │    QemuBackend          │   │
│  │  - Docker/Podman     │    │    - QEMU VMs           │   │
│  │  - Fast iteration    │    │    - High fidelity      │   │
│  └──────────────────────┘    └─────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
```

## Component Details

### Core Layer

#### StorageProvider
- **Purpose**: Manage platform-specific storage paths
- **Key Features**:
  - XDG Base Directory support on Linux
  - %APPDATA% support on Windows
  - Repository URL to filesystem path mapping
  - Hash-based unique identifiers
- **Thread Safety**: Singleton with thread-safe access

#### RepoManager
- **Purpose**: Handle Git repository operations
- **Key Features**:
  - Clone repositories with progress reporting
  - Update existing repositories
  - List managed repositories
  - Validate repository state
- **Dependencies**: Qt Process, StorageProvider
- **Signals**: cloneProgress, cloneFinished, error

#### WorkflowDiscovery
- **Purpose**: Find workflow files in repositories
- **Key Features**:
  - Scan .github/workflows directory
  - Filter .yml and .yaml files
  - Validate workflow files
- **Performance**: Fast directory scanning

#### WorkflowParser
- **Purpose**: Parse GitHub Actions YAML workflows
- **Key Features**:
  - Full workflow structure parsing
  - Support for jobs, steps, needs, env
  - Matrix strategy detection
  - Error reporting with line numbers
- **Dependencies**: yaml-cpp library
- **Data Structures**:
  - Workflow: Top-level workflow representation
  - WorkflowJob: Individual job definition
  - WorkflowStep: Step definition with all attributes

#### JobExecutor
- **Purpose**: Orchestrate workflow execution
- **Key Features**:
  - Topological sort for job dependencies
  - Backend selection (Container/QEMU)
  - Real-time progress reporting
  - Error handling and recovery
- **Signals**: jobStarted, jobFinished, stepStarted, stepFinished, stepOutput
- **State Management**: Thread-safe execution state

#### MatrixStrategy
- **Purpose**: Expand matrix strategies into individual jobs
- **Key Features**:
  - Multi-dimensional matrix support
  - Cartesian product generation
  - Variable substitution in expanded jobs
- **Algorithm**: Efficient combination generation

#### ArtifactManager
- **Purpose**: Manage workflow artifacts locally
- **Key Features**:
  - Upload artifacts to local storage
  - Download artifacts between jobs
  - List available artifacts
  - Automatic compression (planned)
- **Storage**: Cache directory organization

#### CacheManager
- **Purpose**: Implement local caching like actions/cache
- **Key Features**:
  - Key-based cache storage
  - Path pattern support
  - Cache hit/miss reporting
  - Cache invalidation
- **Storage**: Content-addressed by key hash

### Execution Backends

#### ExecutionBackend (Base Class)
- **Purpose**: Abstract interface for execution environments
- **Key Methods**:
  - executeStep(): Run a single workflow step
  - prepareEnvironment(): Set up execution environment
  - cleanup(): Tear down environment
- **Signals**: output, error

#### ContainerBackend
- **Purpose**: Execute workflows in containers
- **Features**:
  - Docker/Podman auto-detection
  - Runner spec to image mapping
  - Container lifecycle management
  - Real-time output streaming
- **Mapping Table**:
  - ubuntu-latest → ubuntu:22.04
  - ubuntu-20.04 → ubuntu:20.04
  - windows-latest → (not supported in containers)

#### QemuBackend
- **Purpose**: Execute workflows in QEMU VMs
- **Features**:
  - QEMU system detection
  - VM image management
  - SSH/guest agent communication (planned)
  - Snapshot support (planned)
- **Requirements**: Pre-built VM images
- **Performance**: Higher startup overhead, better fidelity

### User Interfaces

#### CLI (gwt)
- **Commands**:
  - clone: Clone a repository
  - list: List cloned repositories
  - workflows: Discover workflows
  - run: Execute a workflow
- **Options**:
  - --qemu: Use QEMU backend
  - --event: Specify trigger event
  - --env: Set environment variables
- **Output**: Real-time to stdout/stderr

#### GUI (gwt-gui)
- **Components**:
  - Repository tree view
  - Workflow list view
  - Output console
  - Backend selector
  - Progress indicators
- **Features**:
  - Drag-and-drop support (planned)
  - Syntax highlighting (planned)
  - Job visualization (planned)

## Data Flow

### Workflow Execution Flow

```
1. User triggers workflow run
   ↓
2. WorkflowDiscovery finds workflow file
   ↓
3. WorkflowParser parses YAML → Workflow structure
   ↓
4. MatrixStrategy expands matrix jobs
   ↓
5. JobExecutor resolves job dependencies
   ↓
6. For each job in order:
   a. Select backend (Container/QEMU)
   b. Backend prepares environment
   c. For each step:
      - Execute step in backend
      - Stream output
      - Handle artifacts/cache
   d. Backend cleanup
   ↓
7. Report final status
```

### Storage Organization

```
Platform-specific root/
├── repos/                          # Cloned repositories
│   ├── github.com/
│   │   └── owner/
│   │       └── repo_abc123/       # Hash-suffixed
│   │           ├── .git/
│   │           └── .github/
│   │               └── workflows/
└── cache/                          # Cache and artifacts
    ├── artifacts/                  # Workflow artifacts
    │   └── workflow-id/
    │       └── artifact-name
    └── cache/                      # actions/cache equivalent
        └── key-hash/
            └── cached-files
```

## Technology Stack

### Core Technologies
- **Language**: C++17
- **Framework**: Qt 6.6+
- **Build System**: CMake 3.22+
- **Build Generator**: Ninja
- **Package Manager**: Conan 2.x

### Libraries
- **Qt Modules**:
  - Qt6Core: Core functionality
  - Qt6Widgets: GUI components
- **Third-Party**:
  - yaml-cpp 0.8.0: YAML parsing

### External Tools (Optional)
- **Docker/Podman**: Container backend
- **QEMU**: VM backend
- **Git**: Repository operations

## Design Patterns

### Singleton
- StorageProvider: Ensures consistent path resolution

### Factory
- ExecutionBackend creation based on user selection

### Observer
- Qt signals/slots for event notification
- Progress reporting
- Error handling

### Strategy
- ExecutionBackend: Pluggable execution strategies

### Builder (Implicit)
- WorkflowParser builds complex Workflow structures

## Error Handling

### Error Categories
1. **File System Errors**: Handled with QFile error codes
2. **Git Errors**: Process exit codes and stderr
3. **YAML Errors**: yaml-cpp exceptions, converted to QStringList
4. **Execution Errors**: Step failures, timeouts
5. **Backend Errors**: Container/VM failures

### Error Reporting
- All components emit error signals
- Errors propagate to UI layer
- CLI: stderr output with exit codes
- GUI: Message boxes and status bar

## Performance Considerations

### Optimizations
- Lazy loading of repositories
- Incremental workflow parsing
- Parallel job execution (where dependencies allow)
- Container reuse (planned)
- VM snapshots for faster restarts (planned)

### Scalability
- Handles repositories with 100+ workflows
- Matrix expansion limited by memory
- Concurrent job limit (configurable, planned)

## Security Considerations

### Current State
- No credential management
- Local-only execution
- No network isolation
- File system access unrestricted

### Planned Improvements
- Sandboxed execution
- Secret redaction in logs
- Network policy enforcement
- Resource limits

## Future Enhancements

### Short Term
1. Improve action resolution (actions/checkout, etc.)
2. Add workflow validation
3. Implement dry-run mode
4. Add workflow debugging features

### Long Term
1. Service container support
2. Composite actions
3. Reusable workflows
4. Remote execution
5. Workflow visualization
6. Performance profiling
7. macOS runner support (if feasible)

## Testing Strategy

### Unit Tests (Planned)
- Core component testing
- Parser validation
- Matrix expansion verification

### Integration Tests (Planned)
- End-to-end workflow execution
- Backend compatibility
- Cross-platform validation

### Manual Testing
- Real-world workflow compatibility
- UI/UX validation
- Performance benchmarking

## Build and Deployment

### Build Process
```bash
conan install . --output-folder=build --build=missing
cmake --preset=default
cmake --build build
```

### Installation
- System-wide: Copy to /usr/local/bin or C:\Program Files
- User-local: Copy to ~/.local/bin or %USERPROFILE%\bin
- Portable: Run from build directory

### Dependencies Distribution
- Qt shared libraries (distributed with app or system)
- yaml-cpp (static linkage preferred)
- Runtime: Docker/Podman/QEMU (user-installed)

## Maintenance and Support

### Version Compatibility
- Workflow YAML: GitHub Actions syntax (current)
- Qt: 6.6+ (LTS releases preferred)
- CMake: 3.22+ (Ubuntu 22.04+)

### Platform Support Matrix
| Platform | Container | QEMU | Status |
|----------|-----------|------|--------|
| Linux    | ✅        | ✅   | Full   |
| Windows  | ✅        | ✅   | Full   |
| macOS    | ✅        | ⚠️   | Limited|

## Documentation

### User Documentation
- README.md: Project overview
- BUILD.md: Build instructions
- USAGE.md: User guide
- examples/: Example workflows

### Developer Documentation
- ARCHITECTURE.md: This document
- Code comments: Doxygen-style
- API documentation: Generated from headers

## Contributing

### Code Style
- C++17 standard
- Qt naming conventions
- 4-space indentation
- Comprehensive comments

### Testing Requirements
- New features require tests
- Bug fixes require regression tests
- Platform-specific code requires cross-platform testing

### Review Process
- Code review required
- CI must pass
- Documentation updates required
