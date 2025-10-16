#include "LabelsAndHyperlinks.h"

#include "MainWindow.h"
#include "OptionalFields.h"

#include "cafPdmUiLabelEditor.h"

CAF_PDM_SOURCE_INIT( LabelsAndHyperlinks, "LabelsAndHyperlinks" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
LabelsAndHyperlinks::LabelsAndHyperlinks()
{
    CAF_PDM_InitObject( "Labels And Hyperlinks", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_labelTextField, "LabelTextField", "Label Text this text can be very long", "", "", "" );
    m_labelTextField.uiCapability()->setUiEditorTypeName( caf::PdmUiLabelEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_hyperlinkTextField, "HyperlinkTextField", "" );
    m_hyperlinkTextField.uiCapability()->setUiEditorTypeName( caf::PdmUiLabelEditor::uiEditorTypeName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void LabelsAndHyperlinks::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                 QString                    uiConfigName,
                                                 caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_hyperlinkTextField )
    {
        if ( auto labelEditorAttribute = dynamic_cast<caf::PdmUiLabelEditorAttribute*>( attribute ) )
        {
            labelEditorAttribute->m_linkText =
                "Click <a href=\"dummy\">link</a> to select the <b>Optional Field</b> object.";

            labelEditorAttribute->m_linkActivatedCallback = [this]( const QString& link )
            {
                auto mainWin = MainWindow::instance();
                if ( auto root = mainWin->root() )
                {
                    auto obj = root->descendantsIncludingThisOfType<OptionalFields>();
                    if ( !obj.empty() )
                    {
                        mainWin->setTreeViewSelection( obj.front() );
                    }
                }
            };
        }
    }
}
