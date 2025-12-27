# Building GithubWorkflowTool

## Prerequisites

### Required Dependencies
- CMake 3.22 or higher
- Ninja build system
- Conan 2.x package manager
- Qt 6.6.1 or higher
- C++17 compatible compiler (GCC 9+, Clang 10+, MSVC 2019+)
- Git

### Optional Dependencies
- Docker or Podman (for container backend)
- QEMU (for VM backend)

## Installing Dependencies

### Ubuntu/Debian
```bash
sudo apt update
sudo apt install cmake ninja-build python3-pip git
pip3 install conan
```

### Windows
1. Install CMake from https://cmake.org/download/
2. Install Ninja from https://github.com/ninja-build/ninja/releases
3. Install Python and pip: https://www.python.org/downloads/
4. Install Conan: `pip install conan`
5. Install Git: https://git-scm.com/download/win

## Building the Project

1. Clone the repository:
```bash
git clone https://github.com/johndoe6345789/GithubWorkflowTool.git
cd GithubWorkflowTool
```

2. Configure Conan (first time only):
```bash
conan profile detect --force
```

3. Install dependencies:
```bash
conan install . --output-folder=build --build=missing
```

4. Configure the project:
```bash
cmake --preset=default
```

5. Build:
```bash
cmake --build build
```

## Running the Application

### CLI
```bash
./build/gwt --help
```

### GUI
```bash
./build/gwt-gui
```

## Usage Examples

### Clone a repository
```bash
./build/gwt clone https://github.com/user/repo
```

### List cloned repositories
```bash
./build/gwt list
```

### Discover workflows
```bash
./build/gwt workflows /path/to/cloned/repo
```

### Run a workflow
```bash
./build/gwt run /path/to/cloned/repo /path/to/cloned/repo/.github/workflows/ci.yml
```

### Run with QEMU backend
```bash
./build/gwt run /path/to/cloned/repo /path/to/cloned/repo/.github/workflows/ci.yml --qemu
```

## Development

### Debug Build
```bash
conan install . --output-folder=build-debug --build=missing -s build_type=Debug
cmake --preset=debug
cmake --build build-debug
```

### Clean Build
```bash
rm -rf build build-debug
```

## Troubleshooting

### Qt not found
Make sure Qt 6 is properly installed and Conan can find it:
```bash
conan search qt --remote=conancenter
```

### yaml-cpp not found
Install yaml-cpp through Conan:
```bash
conan install yaml-cpp/0.8.0@
```

### Build errors
Try cleaning and rebuilding:
```bash
rm -rf build
conan install . --output-folder=build --build=missing
cmake --preset=default
cmake --build build
```
