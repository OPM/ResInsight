# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build System

ResInsight uses CMake with multiple build generators supported. The project requires:
- CMake 3.26+ (boost dependency requirement)
- C++23 standard
- GCC 13+ or Clang 19+ (Linux) / MSVC 2022 17.8+ (Windows)
- Python 3.8+
- Qt 6.4+

### Prerequisites and Dependencies

#### Linux (Ubuntu/Debian)

```bash
# Update system and install build tools
sudo apt update
sudo apt install build-essential curl zip unzip tar flex bison ninja-build

# Install Qt6 dependencies
sudo apt install qt6-base-dev qt6-base-private-dev qt6-charts-dev qt6-networkauth-dev libqt6svg6-dev

# Clone repository and initialize submodules
git clone https://github.com/OPM/ResInsight
cd ResInsight
git submodule update --init

# Bootstrap vcpkg for dependency management
ThirdParty/vcpkg/bootstrap-vcpkg.sh
```

#### Windows (Visual Studio 2022)

```powershell
# Install prerequisites:
# - Visual Studio 2022 (17.8+) with C++ workload and CMake tools
# - Qt 6.4+ (via Qt Online Installer or vcpkg)
# - Git for Windows
# - Python 3.8+

# Clone repository and initialize submodules
git clone https://github.com/OPM/ResInsight
cd ResInsight
git submodule update --init

# Bootstrap vcpkg for dependency management
ThirdParty\vcpkg\bootstrap-vcpkg.bat
```

### Build Commands

#### Linux

```bash
# Configure with CMake preset (recommended)
cmake . --preset=linux-base
# Note: Copy CMakeUserPresets-example-linux.json to CMakeUserPresets.json and update Qt paths

# OR configure manually
mkdir build && cd build
cmake .. -GNinja

# Build the project
cd build && ninja
# OR from workspace root
ninja -C build

# Build specific targets
ninja -C build ResInsight
ninja -C build extract-projectfile-versions
```

#### Windows (Visual Studio 2022)

```powershell
# Configure with CMake preset (recommended)
cmake . --preset=x64-relwithdebinfo
# Note: Copy CMakeUserPresets-example-windows.json to CMakeUserPresets.json and update Qt paths

# OR using Visual Studio 2022 CMake integration:
# 1. Open Visual Studio 2022
# 2. File -> Open -> Folder -> Select ResInsight root directory
# 3. VS will automatically detect CMakeLists.txt and configure the project
# 4. Select x64-relwithdebinfo configuration from the dropdown
# 5. Build -> Build All

# Build the project from command line
cmake --build --preset x64-relwithdebinfo

# Build specific targets
cmake --build --preset x64-relwithdebinfo --target ResInsight
cmake --build --preset x64-relwithdebinfo --target extract-projectfile-versions
```

### Build Presets

Available CMake presets in `CMakePresets.json`:
- `ninja`: Base configuration with vcpkg toolchain, unit tests enabled, warnings as errors
- `linux-base`: Inherits from ninja, includes Qt path configuration (Linux)
- `x64-release`: Release build configuration
- `x64-relwithdebinfo`: Release with debug info configuration (Windows)
- `x64-debug`: Debug build configuration (Windows)

Create `CMakeUserPresets.json` from appropriate example file:
- Linux: Copy `CMakeUserPresets-example-linux.json` and update `CMAKE_PREFIX_PATH` for Qt installation
- Windows: Copy `CMakeUserPresets-example-windows.json` and update Qt paths, Python executable, and ODB API paths

### Code Style and Formatting

- **clang-format**: Configuration in `.clang-format`

Use clang-format-19 to enforce style.

### Test Commands

The project uses CTest for testing:

#### Linux

```bash
# From build directory
ctest
# OR
ninja test

# Run specific test suites
ctest -R "UnitTests"
ctest -R "opm-parser-tests"
ctest -R "roffcpp-tests"
ctest -R "regression-analysis-tests"
```

#### Windows

```powershell
# From build directory
ctest -C RelWithDebInfo
# OR using CMake preset
cmake --build --preset x64-relwithdebinfo --target test

# Run specific test suites
ctest -R "UnitTests" -C RelWithDebInfo
ctest -R "opm-parser-tests" -C RelWithDebInfo
ctest -R "roffcpp-tests" -C RelWithDebInfo
ctest -R "regression-analysis-tests" -C RelWithDebInfo
```

## Architecture Overview

ResInsight is a 3D visualization and post-processing tool for reservoir simulation data built with a modular architecture:

### Core Framework Structure

- **Visualization Framework (Fwk/VizFwk/)**: Core 3D rendering using Qt/OpenGL
  - LibCore, LibGeometry, LibRender, LibViewing, LibGuiQt
  
- **Application Framework (Fwk/AppFwk/)**: UI and data management framework
  - Project Data Model (PDM) system for serialization and UI generation
  - Command framework for undo/redo operations
  - User interface components built on Qt

- **Application Code (ApplicationLibCode/)**: Domain-specific functionality
  - Commands/, ResultStatisticsCache/, GeoMech/ modules
  - Integration with Eclipse, ODB, and other reservoir simulation formats

### Key Dependencies

- **Qt6**: Application framework and UI (Core, Gui, OpenGL, Network, Widgets)
- **Third-party libraries**: resdata (ERT), OPM libraries, qwt plotting, OpenVDS seismic data
- **vcpkg**: Package manager for most dependencies

### Data Sources Supported

- Eclipse binary files (*.GRID, *.EGRID with *.INIT, *.XNNN, *.UNRST)
- ABAQUS ODB files (when RESINSIGHT_ODB_API_DIR is configured)
- OpenVDS seismic data
- Various well and completion data formats

### Build Configuration Options

Key CMake options:
- `RESINSIGHT_ENABLE_GRPC`: Enable gRPC scripting framework
- `RESINSIGHT_USE_ODB_API`: Enable ABAQUS ODB support
- `RESINSIGHT_ENABLE_UNITY_BUILD`: Experimental build speedup
- `RESINSIGHT_ENABLE_HDF5`: Enable HDF5 library support

### Python Integration

ResInsight includes Python integration via gRPC when `RESINSIGHT_ENABLE_GRPC=ON`:
- Python API modules in GrpcInterface/Python/
- Two-way data exchange capabilities
- Script automation support

### Python tests

#### Linux

```bash
# Make a virtual environment: 
python3 -m venv venv-claude

# Start the virtual env:
source venv-claude/bin/activate

# Install pytest
pip install pytest

# Run the tests (e.g all tests in tests_polygons.py):
cd GrpcInterface/Python/rips && source /workspace/venv-claude/bin/activate && RESINSIGHT_EXECUTABLE=build-claude/ResInsight python -m pytest tests/test_polygons.py --console
```

#### Windows

```powershell
# Make a virtual environment: 
python -m venv venv-claude

# Start the virtual env:
venv-claude\Scripts\Activate.ps1

# Install pytest
pip install pytest

# Run the tests (e.g all tests in tests_polygons.py):
cd GrpcInterface\Python\rips
$env:RESINSIGHT_EXECUTABLE="..\..\..\..\build\RelWithDebInfo\ResInsight.exe"
python -m pytest tests/test_polygons.py --console
```

### Python formatting and code style checks using ruff

```bash
# Format source code
python -m ruff format test_polygons.py 
# Check code style
python -m ruff check --fix test_polygons.py
```

### Making PDM Objects Scriptable for Python GRPC Interface

To make a PDM (Project Data Model) object available in the Python GRPC interface, you need to make it "scriptable":

#### 1. Convert PDM Object to Scriptable

**In the header file (.h):**
- No changes needed to the class declaration
- The object inherits from `caf::PdmObject` as usual

**In the source file (.cpp):**
```cpp
// Add required includes
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

// Change object initialization
CAF_PDM_InitScriptableObject("Display Name", ":/Icon.png", "", "ScriptKeyword");
```

#### 2. Convert PDM Fields to Scriptable

**Change field initialization from:**
```cpp
CAF_PDM_InitField(&m_fieldName, "FieldName", defaultValue, "Display Name");
```

**To:**
```cpp
CAF_PDM_InitScriptableField(&m_fieldName, "ScriptFieldName", defaultValue, "Display Name");
```

#### 3. Field Naming Conventions for Python

- Use camelCase for script field names (e.g., "StartMd", "EndMd")
- Python generator converts to snake_case automatically (start_md, end_md)
- Avoid abbreviations like "MD" - use "Md" instead

#### 4. Class Keyword Requirements

The class keyword used in `CAF_PDM_SOURCE_INIT` must be unique and descriptive:
```cpp
CAF_PDM_SOURCE_INIT(RimClassName, "ClassKeyword");
```

#### 5. Build and Python Generation

After making objects scriptable:
1. Build the project: `ninja -C build`
2. Python classes are automatically generated in `build/Python/rips/generated/generated_classes.py`
3. The scriptable class will appear as a Python class with appropriate attributes

#### 6. GRPC Method Integration

To add methods that return scriptable objects:
```cpp
// In GRPC interface method
QString RimcClassName_method::classKeywordReturnedType() const
{
    return RimTargetClass::classKeywordStatic();
}
```

#### Example: DiameterRoughnessInterval

**Before (not scriptable):**
```cpp
CAF_PDM_InitObject("Diameter Roughness Interval", ":/Icon.png");
CAF_PDM_InitField(&m_startMD, "StartMD", 0.0, "Start MD");
```

**After (scriptable):**
```cpp
CAF_PDM_InitScriptableObject("Diameter Roughness Interval", ":/Icon.png", "", "DiameterRoughnessInterval");
CAF_PDM_InitScriptableField(&m_startMD, "StartMd", 0.0, "Start MD");
```

This enables the class to be used from Python:
```python
interval = completions_settings.add_diameter_roughness_interval(start_md=100, end_md=200)
print(f"Start: {interval.start_md}, End: {interval.end_md}")
```

## Development Notes

- **Version**: Current version defined in `ResInsightVersion.cmake` (2025.04.4-dev)
- **Git Workflow**: Main development on `dev` branch, `master` branch for stable releases
- **Dependencies**: Extensive third-party integration via git submodules in `ThirdParty/`
- **Build Output**: Goes to `CMAKE_RUNTIME_OUTPUT_DIRECTORY` (CMAKE_CURRENT_BINARY_DIR)
- **Qt Integration**: Uses Qt's resource system (.qrc files) for UI resources
- **Entry Point**: Main executable and supporting tools built from `ApplicationExeCode/`
- **Testing**: Cross-platform automated testing on RHEL, Ubuntu, and Windows 11
- **Build Configuration**: Key option `RESINSIGHT_TREAT_WARNINGS_AS_ERRORS` available in CMake presets

## Common File Locations

- Main CMake configuration: `/workspace/CMakeLists.txt`
- Application entry point: `ApplicationExeCode/RiaMain.cpp`
- Version info: `ResInsightVersion.cmake`
- Third-party dependencies: `ThirdParty/` with individual CMakeLists.txt files
- Python API: `GrpcInterface/Python/`

- When creating a git commit for a feature request use the issue number at the start of the title, e.g "#12773 Python: Add API for creating valve templates"
- When creating a commit use git conventions. Skip attributing claude-code as author (although you were helpful).
- Always run python formatting/check on changed files before commits.