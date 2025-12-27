cmake_minimum_required(VERSION 3.22)
project(GithubWorkflowTool VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Conan integration
include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake OPTIONAL)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
qt_standard_project_setup()

# Find yaml-cpp for workflow parsing
find_package(yaml-cpp REQUIRED)

# Source files
set(CORE_SOURCES
    src/core/StorageProvider.cpp
    src/core/RepoManager.cpp
    src/core/WorkflowDiscovery.cpp
    src/core/WorkflowParser.cpp
    src/core/JobExecutor.cpp
    src/core/MatrixStrategy.cpp
    src/core/ArtifactManager.cpp
    src/core/CacheManager.cpp
)

set(BACKEND_SOURCES
    src/backends/ContainerBackend.cpp
    src/backends/QemuBackend.cpp
)

set(CLI_SOURCES
    src/cli/main.cpp
    src/cli/CommandHandler.cpp
)

set(GUI_SOURCES
    src/gui/MainWindow.cpp
    src/gui/WorkflowView.cpp
    src/gui/JobView.cpp
    src/gui/main.cpp
)

# Core library
add_library(gwt_core STATIC ${CORE_SOURCES} ${BACKEND_SOURCES})
target_include_directories(gwt_core PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(gwt_core PUBLIC 
    Qt6::Core
    yaml-cpp
)

# CLI executable
add_executable(gwt_cli ${CLI_SOURCES})
target_link_libraries(gwt_cli PRIVATE gwt_core Qt6::Core)
set_target_properties(gwt_cli PROPERTIES OUTPUT_NAME "gwt")

# GUI executable
add_executable(gwt_gui ${GUI_SOURCES})
target_link_libraries(gwt_gui PRIVATE gwt_core Qt6::Core Qt6::Widgets)
set_target_properties(gwt_gui PROPERTIES 
    OUTPUT_NAME "gwt-gui"
    WIN32_EXECUTABLE TRUE
)

# Installation
install(TARGETS gwt_cli gwt_gui
    RUNTIME DESTINATION bin
)
