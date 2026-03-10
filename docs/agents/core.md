# Core Agent Guidelines

This document provides core guidelines that all AI agents working on ResInsight should follow.

## Project Overview

ResInsight is a 3D visualization and post-processing tool for reservoir simulation data built with a modular architecture.

## Architecture Overview

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

## Git

### Commit Conventions
- When creating a git commit for a feature request use the issue number at the start of the title, e.g "#12773 Python: Add API for creating valve templates"
- When creating a commit use git conventions
- Always run python formatting/check on changed files before commits

### PR Conventions
- Do not include a test plan in the PR description

## Making PDM Objects Scriptable for Python GRPC Interface

To make a PDM (Project Data Model) object available in the Python GRPC interface, you need to make it "scriptable":

### 1. Convert PDM Object to Scriptable

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

### 2. Convert PDM Fields to Scriptable

**Change field initialization from:**
```cpp
CAF_PDM_InitField(&m_fieldName, "FieldName", defaultValue, "Display Name");
```

**To:**
```cpp
CAF_PDM_InitScriptableField(&m_fieldName, "ScriptFieldName", defaultValue, "Display Name");
```

### 3. Field Naming Conventions for Python

- Use camelCase for script field names (e.g., "StartMd", "EndMd")
- Python generator converts to snake_case automatically (start_md, end_md)
- Avoid abbreviations like "MD" - use "Md" instead

### 4. Class Keyword Requirements

The class keyword used in `CAF_PDM_SOURCE_INIT` must be unique and descriptive:
```cpp
CAF_PDM_SOURCE_INIT(RimClassName, "ClassKeyword");
```

### 5. Build and Python Generation

After making objects scriptable:
1. Build the project: `ninja -C build`
2. Python classes are automatically generated in `build/Python/rips/generated/generated_classes.py`
3. The scriptable class will appear as a Python class with appropriate attributes

### 6. GRPC Method Integration

To add methods that return scriptable objects:
```cpp
// In GRPC interface method
QString RimcClassName_method::classKeywordReturnedType() const
{
    return RimTargetClass::classKeywordStatic();
}
```

### Example: DiameterRoughnessInterval

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

## Field Validation in Python GRPC Interface

Python field updates are automatically validated when calling `obj.update()`. This ensures data integrity by preventing invalid values from being set.

### Validation Types

1. **Type Validation**: Ensures field value types match (int, float, string, etc.)
   - Parsing errors are caught during type conversion
   - Example: Setting a string "hello" to an integer field is rejected

2. **Range Validation**: Checks values against min/max constraints defined with `setRange()`, `setMinValue()`, or `setMaxValue()`
   - Fields with range constraints reject out-of-bounds values
   - Example: A field with `setMinValue(0)` rejects negative values

3. **Object Validation**: Validates cross-field constraints via `validate()` override
   - Custom business logic can enforce relationships between fields
   - Example: Ensuring end date is after start date

### Setting Field Ranges in C++

To add range validation to a PDM field:

```cpp
RimMyClass::RimMyClass()
{
    CAF_PDM_InitScriptableField(&m_percentage, "Percentage", 50.0, "Percentage");
    m_percentage.setRange(0.0, 100.0);  // Validates 0-100 range

    CAF_PDM_InitScriptableField(&m_count, "Count", 1, "Count");
    m_count.setMinValue(0);  // Only validates minimum
    m_count.setMaxValue(100);  // Only validates maximum

    CAF_PDM_InitScriptableField(&m_wellBoreFluidPvtTable, "WellBoreFluidPvtTable", 0, "Wellbore Fluid PVT Table");
    m_wellBoreFluidPvtTable.setMinValue(0);  // Must be non-negative
}
```

### Python Behavior

Invalid values are **rejected** and field values remain unchanged:

```python
# Example: Setting invalid value
completions_settings = well_path.completion_settings()
completions_settings.well_bore_fluid_pvt_table = -5  # Invalid: below minimum of 0

try:
    completions_settings.update()  # Triggers validation
except Exception as e:
    print(f"Validation failed: {e}")
    # Output:
    # Validation failed: <_InactiveRpcError of RPC that terminated with:
    #     status = StatusCode.INVALID_ARGUMENT
    #     details = "Validation failed for WellPathCompletionSettings:
    #     WellBoreFluidPvtTable: Value -5 is below minimum 0"
    # >

# The field retains its previous valid value (rollback occurred)
current_value = well_path.completion_settings().well_bore_fluid_pvt_table
print(f"Value remains: {current_value}")  # Previous valid value
```

### Error Handling

Validation errors raise gRPC exceptions with descriptive messages:

```python
try:
    obj.update()
except Exception as e:
    error_msg = str(e)
    if "Validation failed" in error_msg:
        # Handle validation error
        print("Please correct the following fields:")
        print(error_msg)
    else:
        # Other error
        raise
```

### Multiple Field Errors

All validation errors are aggregated and returned together:

```python
# Set multiple invalid values
settings.field1 = invalid_value1
settings.field2 = invalid_value2

try:
    settings.update()
except Exception as e:
    # Error message lists all validation failures:
    # "Validation failed for ClassName:
    # field1: Value ... exceeds maximum ...
    # field2: Value ... is below minimum ..."
    pass
```

### Validation Guarantees

- **Atomic Updates**: If any field fails validation, no fields are updated (all-or-nothing)
- **Rollback**: Failed updates don't modify field values - they remain at their previous valid state
- **Type Safety**: Cannot assign values of wrong type (string to int, etc.)
- **Clear Messages**: Error messages include field names and specific validation failures

### Implementation Notes

- Validation occurs automatically on `update()` - no manual validation needed
- Built on commit f342ca77572c840eadf22fe0a3fd2935c62fd04b validation infrastructure
- Uses C++23 `std::expected<void, QString>` for clean error handling
- Returns `grpc::INVALID_ARGUMENT` status with validation details

## PDM UI Editor Attributes - Modern setAttribute Pattern

Note: Always use () when accessing a field's value. This is required for proper type management. Use `m_myTextField()` instead of `m_myTextField`.

The CAF (Command Application Framework) PDM (Project Data Model) system has migrated from the old `defineEditorAttribute` pattern to a modern `setAttribute` pattern using type-safe Keys structs.

### New Pattern: setAttribute with Keys Struct

**Setting attributes using the new pattern:**

```cpp
// Modern approach - use Keys constants with implicit type deduction
m_comboBoxField.uiCapability()->setAttribute( caf::PdmUiComboBoxEditor::Keys::ADJUST_WIDTH_TO_CONTENTS, true );
m_comboBoxField.uiCapability()->setAttribute( caf::PdmUiComboBoxEditor::Keys::MINIMUM_CONTENTS_LENGTH, 15 );
m_comboBoxField.uiCapability()->setAttribute( caf::PdmUiComboBoxEditor::Keys::BUTTON_TEXT, "Click Me" );
```

### Benefits of the New Pattern

1. **Type Safety**: Compile-time checking of attribute names prevents typos
2. **IDE Support**: Auto-completion for attribute keys
3. **Self-Documenting**: Keys struct clearly shows available attributes for each editor
4. **Maintainable**: Single source of truth for attribute names
5. **Validation**: Automatic warnings for unsupported attributes

### Migration from Old Pattern

**Old pattern (deprecated):**
```cpp
// In defineEditorAttribute() override
auto* myAttr = dynamic_cast<caf::PdmUiComboBoxEditorAttribute*>( attribute );
if ( myAttr )
{
    myAttr->adjustWidthToContents = true;
    myAttr->minimumContentsLength = 15;
}
```

**New pattern:**
```cpp
// In field initialization or elsewhere
m_field.uiCapability()->setAttribute( caf::PdmUiComboBoxEditor::Keys::ADJUST_WIDTH_TO_CONTENTS, true );
m_field.uiCapability()->setAttribute( caf::PdmUiComboBoxEditor::Keys::MINIMUM_CONTENTS_LENGTH, 15 );
```

### Common Attribute Examples

```cpp
// Line Editor - notify on text changes
m_textField.uiCapability()->setAttribute( caf::PdmUiLineEditor::Keys::NOTIFY_WHEN_TEXT_IS_EDITED, true );

// Label Editor - hyperlink with callback
m_labelField.uiCapability()->setAttribute( caf::PdmUiLabelEditor::Keys::LINK_TEXT, 
                                           "Click <a href='link'>here</a>" );
std::function<void(const QString&)> callback = [](const QString& link) { /* handler */ };
m_labelField.uiCapability()->setAttribute( caf::PdmUiLabelEditor::Keys::LINK_ACTIVATED_CALLBACK,
                                          QVariant::fromValue( callback ) );

// List Editor - set height hint
m_listField.uiCapability()->setAttribute( caf::PdmUiListEditor::Keys::HEIGHT_HINT, 150 );

// Push Button - set button text
m_buttonField.uiCapability()->setAttribute( caf::PdmUiPushButtonEditor::Keys::BUTTON_TEXT, "Execute" );

// Combo Box - enable previous/next buttons
m_comboField.uiCapability()->setAttribute( caf::PdmUiComboBoxEditor::Keys::SHOW_PREVIOUS_AND_NEXT_BUTTONS, true );
```

### Backward Compatibility

The old `defineEditorAttribute` pattern still works but is discouraged for new code. Both patterns can coexist during migration, with the new `setAttribute` pattern taking precedence when both are used.
