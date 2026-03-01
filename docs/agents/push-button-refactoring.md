# Push Button Refactoring: PdmField<bool> → PdmUiButton

## Overview

Push buttons in the PDM UI can be implemented two ways:
- **Old pattern**: `caf::PdmField<bool>` with `PdmUiPushButtonEditor` (deprecated for new code)
- **New pattern**: `caf::PdmUiButton` added via `uiOrdering.addNewButton()` in `defineUiOrdering`

The new pattern is preferred because it requires less boilerplate, eliminates the field lifecycle, and keeps the action logic co-located with the UI layout.

## Old Pattern (to be replaced)

### Header
```cpp
caf::PdmField<bool> m_myButton;
```

### Constructor
```cpp
CAF_PDM_InitField( &m_myButton, "MyButton", false, "" );
caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_myButton );
m_myButton.xmlCapability()->disableIO(); // if not serialized
```

### defineEditorAttribute
```cpp
else if ( field == &m_myButton )
{
    if ( auto* attr = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute ) )
        attr->m_buttonText = "Click Me";
}
```

### defineUiOrdering
```cpp
group->add( &m_myButton );
```

### fieldChangedByUi
```cpp
else if ( changedField == &m_myButton )
{
    doSomething();
    m_myButton = false; // reset
}
```

### Required include
```cpp
#include "cafPdmUiPushButtonEditor.h"
```

## New Pattern (PdmUiButton)

### Header
No field declaration needed.

### Constructor
No init call needed.

### defineUiOrdering
```cpp
group->addNewButton( "Click Me", [this]() { doSomething(); } );
```

With layout options:
```cpp
group->addNewButton( "Click Me", [this]() { doSomething(); }, { .newRow = false, .totalColumnSpan = 1 } );
```

### Required include
```cpp
#include "cafPdmUiButton.h"
```

## Migration Steps

1. **Header**: Remove `caf::PdmField<bool> m_myButton;`
2. **Constructor**: Remove `CAF_PDM_InitField`, `configureEditorLabelHidden`, and `disableIO` lines
3. **defineEditorAttribute**: Remove the `else if ( field == &m_myButton )` block
4. **defineUiOrdering**: Replace `group->add( &m_myButton )` with `group->addNewButton( "Text", [this]() { ... } )`
5. **fieldChangedByUi**: Remove the `else if ( changedField == &m_myButton )` block (move logic into lambda)
6. **Includes**: Replace `cafPdmUiPushButtonEditor.h` with `cafPdmUiButton.h` (if no other push button fields remain)

## Files Already Migrated

- `ApplicationLibCode/Application/RiaPreferences.cpp` — migrated `m_deleteOsduToken`, `m_deleteSumoToken`, `m_importPreferences`, `m_exportPreferences`

## Files Still Using Old Pattern

Run the following to find remaining files:
```
grep -rl "configureEditorLabelHidden\|configureEditorLabelLeft" ApplicationLibCode/
```
