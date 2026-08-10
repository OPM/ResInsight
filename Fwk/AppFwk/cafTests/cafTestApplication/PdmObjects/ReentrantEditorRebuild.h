#pragma once

#include "cafPdmField.h"
#include "cafPdmObject.h"

//==================================================================================================
/// Reproduction of https://github.com/OPM/ResInsight/issues/14505
///
/// Rebuilding the property editors from inside fieldChangedByUi() destroys widgets that Qt is
/// still executing an event handler on, giving a use-after-free in QLineEdit::focusOutEvent.
///
/// Steps to reproduce:
///   1. Select this object in the tree view so the property editor is shown.
///   2. Type a value into "Realization Filter".
///   3. Press Tab. Do not press Enter, the focus-out path is the dangerous one. Enter is handled
///      by PdmUiLineEditor::eventFilter, which calls slotEditingFinished() outside focusOutEvent.
///
/// Uncheck "Rebuild Layout On Change" to get the same edit without the reentrant rebuild.
//==================================================================================================
class ReentrantEditorRebuild : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    ReentrantEditorRebuild();

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

private:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    void restoreFilterGroup();

private:
    caf::PdmField<QString> m_realizationFilter;
    caf::PdmField<QString> m_statusField;
    caf::PdmField<bool>    m_rebuildLayoutOnChange;

    bool m_hideFilterGroup;
};
