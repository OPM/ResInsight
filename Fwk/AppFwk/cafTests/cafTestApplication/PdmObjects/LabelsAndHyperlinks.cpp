#include "LabelsAndHyperlinks.h"

#include "../MainWindow.h"
#include "OptionalFields.h"

#include "cafPdmUiButton.h"
#include "cafPdmUiLabelEditor.h"

#include <QMessageBox>

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

    // Set attributes using new map-based system
    m_hyperlinkTextField.uiCapability()->setAttribute( caf::PdmUiLabelEditor::Keys::LINK_TEXT,
                                                       "Click <a href=\"dummy\">link</a> to select the "
                                                       "<b>Optional Field</b> object." );

    std::function<void( const QString& )> callback = [this]( const QString& link )
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

    m_hyperlinkTextField.uiCapability()->setAttribute( caf::PdmUiLabelEditor::Keys::LINK_ACTIVATED_CALLBACK,
                                                       QVariant::fromValue( callback ) );

    CAF_PDM_InitField( &m_showButton, "ShowButton", true, "Show Fieldless Button" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void LabelsAndHyperlinks::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.addNewLabel( "This is a standalone label without PDM field connection" );
    uiOrdering.addNewLabel( "Labels can display informational text in the GUI" );

    if ( m_showButton )
    {
        // Button with text and lambda callback
        uiOrdering.addNewButton( "Click Me!",
                                 []()
                                 {
                                     // This lambda will be executed when the button is clicked
                                     auto msgBox = new QMessageBox();
                                     msgBox->setWindowTitle( "Button Clicked" );
                                     msgBox->setText( "Hello from the button callback!" );
                                     msgBox->setAttribute( Qt::WA_DeleteOnClose );
                                     msgBox->show();
                                 } );

        // Button with icon and callback
        auto* iconButton = uiOrdering.addNewButton( "Icon Button",
                                                    []()
                                                    {
                                                        auto msgBox = new QMessageBox();
                                                        msgBox->setWindowTitle( "Icon Button" );
                                                        msgBox->setText( "This button has an icon!" );
                                                        msgBox->setAttribute( Qt::WA_DeleteOnClose );
                                                        msgBox->show();
                                                    } );
        iconButton->setIconFromResourceString( ":/cafCommandFeatures/Delete.svg" );
    }
    else
    {
        // Button with text and lambda callback
        uiOrdering.addNewButton( "Click Me too!",
                                 []()
                                 {
                                     // This lambda will be executed when the button is clicked
                                     auto msgBox = new QMessageBox();
                                     msgBox->setWindowTitle( "Button Clicked" );
                                     msgBox->setText( "Hello from the button callback!" );
                                     msgBox->setAttribute( Qt::WA_DeleteOnClose );
                                     msgBox->show();
                                 } );
    }

    uiOrdering.add( &m_labelTextField );
    uiOrdering.add( &m_hyperlinkTextField );

    uiOrdering.addNewLabel( "Another label at the bottom demonstrating multiple labels" );
}
