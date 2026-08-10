#include "ReentrantEditorRebuild.h"

#include "cafPdmUiLineEditor.h"
#include "cafPdmUiOrdering.h"

CAF_PDM_SOURCE_INIT( ReentrantEditorRebuild, "ReentrantEditorRebuild" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ReentrantEditorRebuild::ReentrantEditorRebuild()
    : m_hideFilterGroup( false )
{
    CAF_PDM_InitObject( "Reentrant Editor Rebuild", "", "", "" );

    CAF_PDM_InitField( &m_realizationFilter,
                       "RealizationFilter",
                       QString(),
                       "Realization Filter",
                       "",
                       "Type a value here and press Tab",
                       "" );

    CAF_PDM_InitField( &m_statusField, "Status", QString( "Type into Realization Filter, then press Tab" ), "Status", "", "", "" );
    m_statusField.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_rebuildLayoutOnChange,
                       "RebuildLayoutOnChange",
                       true,
                       "Rebuild Layout On Change",
                       "",
                       "Rebuild all property editors from inside fieldChangedByUi()",
                       "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ReentrantEditorRebuild::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                               const QVariant&            oldValue,
                                               const QVariant&            newValue )
{
    if ( changedField == &m_realizationFilter )
    {
        m_statusField = "Committed value: " + m_realizationFilter();

        if ( m_rebuildLayoutOnChange )
        {
            // Drop the group that parents the line edit, and rebuild every property editor while
            // still inside QLineEdit::focusOutEvent. This mirrors
            // RimSumoDataSource::fieldChangedByUi() -> RimSummaryEnsembleSumo::onRealizationSelectionChanged().
            m_hideFilterGroup = true;

            updateAllRequiredEditors();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ReentrantEditorRebuild::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_statusField );
    uiOrdering.add( &m_rebuildLayoutOnChange );

    if ( !m_hideFilterGroup )
    {
        // The line edit lives inside this group. When both the field and the group disappear from
        // the ui ordering, PdmUiFormLayoutObjectEditor::configureAndUpdateUi() first deletes the
        // field editor, whose destructor only calls deleteLater() on the QLineEdit, and then
        // destroys the parent QMinimizePanel with a raw delete. ~QWidget destroys its children
        // immediately, so the QLineEdit is gone before the pending deleteLater() can run.
        auto group = uiOrdering.addNewGroup( "Filter" );
        group->add( &m_realizationFilter );
    }

    uiOrdering.addNewButton( "Restore Filter Group", [this]() { restoreFilterGroup(); } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ReentrantEditorRebuild::restoreFilterGroup()
{
    m_hideFilterGroup = false;

    updateAllRequiredEditors();
}
