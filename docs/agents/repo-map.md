# Repository Map

This document provides an overview of the ResInsight repository structure and key file locations.

## Top-Level Directory Structure

```
ResInsight/
├── ApplicationExeCode/      # Main executable entry points
│   └── RiaMain.cpp          # Application entry point
├── ApplicationLibCode/      # Domain-specific functionality
│   ├── Commands/            # Command framework implementations
│   ├── ResultStatisticsCache/
│   └── GeoMech/            # Geomechanics modules
├── Fwk/                     # Framework libraries
│   ├── VizFwk/             # Visualization framework (Qt/OpenGL)
│   │   ├── LibCore
│   │   ├── LibGeometry
│   │   ├── LibRender
│   │   ├── LibViewing
│   │   └── LibGuiQt
│   └── AppFwk/             # Application framework
│       ├── PDM system       # Project Data Model for serialization
│       ├── Command framework # Undo/redo operations
│       └── UI components    # Qt-based user interface
├── GrpcInterface/           # gRPC integration
│   └── Python/             # Python API
│       └── rips/           # Python package
├── ThirdParty/             # Third-party dependencies (git submodules)
│   └── vcpkg/              # vcpkg package manager
├── TestModels/             # Test data and models
├── cmake/                  # CMake modules and utilities
├── docs/                   # Documentation
│   ├── agents/             # Agent-specific documentation
│   └── class-diagrams/     # PlantUML class diagrams
├── scripts/                # Build and utility scripts
├── tooling/                # Development tools
├── patches/                # Patches for dependencies
├── CMakeLists.txt         # Main CMake configuration
├── CMakePresets.json      # CMake build presets
├── ResInsightVersion.cmake # Version information
└── vcpkg.json             # vcpkg dependencies
```

## Key Files

### Build Configuration
- `CMakeLists.txt` - Main CMake configuration
- `CMakePresets.json` - Predefined build configurations
- `CMakeUserPresets-example-linux.json` - Example user presets for Linux
- `CMakeUserPresets-example-windows.json` - Example user presets for Windows
- `vcpkg.json` - vcpkg dependency manifest
- `vcpkg-configuration.json` - vcpkg configuration

### Version and Documentation
- `ResInsightVersion.cmake` - Version definitions (2025.04.4-dev)
- `README.md` - Project overview and getting started
- `COPYING` - License information
- `docs/agents/` - Agent documentation
  - `core.md` - Core agent guidelines
  - `build.md` - Build system documentation
  - `coding-style.md` - Coding style guidelines
  - `repo-map.md` - This file

### Code Formatting
- `.clang-format` - clang-format configuration for C++ code
- `.codespellrc` - codespell configuration
- `.codespell-ignore` - codespell ignore list
- `.misspell-fixer.ignore` - misspell-fixer ignore list

### Source Code Organization

#### Visualization Framework (Fwk/VizFwk/)
Core 3D rendering using Qt/OpenGL with libraries for:
- Geometry processing
- Rendering pipeline
- View management
- Qt GUI integration

#### Application Framework (Fwk/AppFwk/)
- **PDM System**: Project Data Model for:
  - Object serialization
  - UI generation
  - Field validation
- **Command Framework**: Undo/redo operations
- **UI Components**: Qt-based widgets and dialogs

#### Application Code (ApplicationLibCode/)
Domain-specific functionality including:
- Eclipse file format integration
- ODB (ABAQUS) support
- Well path and completion management
- Grid and geometry processing
- Result statistics and caching

#### Python Integration (GrpcInterface/Python/)
- gRPC server implementation
- Python API (rips package)
- Generated Python classes from PDM objects
- Test suite for Python API

## Data File Formats

ResInsight supports various reservoir simulation file formats:
- Eclipse binary: `*.GRID`, `*.EGRID`, `*.INIT`, `*.XNNN`, `*.UNRST`
- ABAQUS ODB files (when `RESINSIGHT_ODB_API_DIR` configured)
- OpenVDS seismic data
- Well and completion data formats

## Build Outputs

Build artifacts are placed in:
- Linux: `build/` directory (or preset-specific directory)
- Windows: `build/RelWithDebInfo/`, `build/Debug/`, etc.
- Python generated classes: `build/Python/rips/generated/generated_classes.py`

## Testing

- Unit tests: Various test executables
- Integration tests: CTest-based test suites
- Python tests: `GrpcInterface/Python/rips/tests/`
- Test data: `TestModels/` directory
