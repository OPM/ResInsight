# Build System Documentation

ResInsight uses CMake with multiple build generators supported. The project requires:
- CMake 3.26+ (boost dependency requirement)
- C++23 standard
- GCC 13+ or Clang 19+ (Linux) / MSVC 2022 17.8+ (Windows)
- Python 3.8+
- Qt 6.4+

## Prerequisites and Dependencies

### Linux (Ubuntu/Debian)

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

### Windows (Visual Studio 2022)

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

## Build Commands

### Linux

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

### Windows (Visual Studio 2022)

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

## Build Presets

Available CMake presets in `CMakePresets.json`:
- `ninja`: Base configuration with vcpkg toolchain, unit tests enabled, warnings as errors
- `linux-base`: Inherits from ninja, includes Qt path configuration (Linux)
- `x64-release`: Release build configuration
- `x64-relwithdebinfo`: Release with debug info configuration (Windows)
- `x64-debug`: Debug build configuration (Windows)

Create `CMakeUserPresets.json` from appropriate example file:
- Linux: Copy `CMakeUserPresets-example-linux.json` and update `CMAKE_PREFIX_PATH` for Qt installation
- Windows: Copy `CMakeUserPresets-example-windows.json` and update Qt paths, Python executable, and ODB API paths

## Test Commands

The project uses CTest for testing:

### Linux

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

### Windows

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

## Python Tests

### Linux

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

### Windows

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
