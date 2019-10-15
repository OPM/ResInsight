#include "RicExportContourMapToAsciiUi.h"

#include "RiaApplication.h"

#include "cafPdmUiFilePathEditor.h"

CAF_PDM_SOURCE_INIT( RicExportContourMapToAsciiUi, "RicExportContourMapToAsciiUi" );

RicExportContourMapToAsciiUi::RicExportContourMapToAsciiUi()
{
    CAF_PDM_InitObject( "Export Contour Map to Text", "", "", "" );

    CAF_PDM_InitField( &m_exportFileName, "ExportFileName", QString(), "Export File Name", "", "", "" );
    m_exportFileName.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_exportLocalCoordinates, "ExportLocalCoordinates", false, "Export Local Coordinates", "", "", "" );
    CAF_PDM_InitField( &m_undefinedValueLabel, "UndefinedValueLabel", QString( "NaN" ), "Undefined Value Label", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicExportContourMapToAsciiUi::exportFileName() const
{
    return m_exportFileName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportContourMapToAsciiUi::setExportFileName( const QString& exportFileName )
{
    m_exportFileName = exportFileName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportContourMapToAsciiUi::exportLocalCoordinates() const
{
    return m_exportLocalCoordinates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicExportContourMapToAsciiUi::undefinedValueLabel() const
{
    return m_undefinedValueLabel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportContourMapToAsciiUi::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                          QString                    uiConfigName,
                                                          caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_exportFileName )
    {
        caf::PdmUiFilePathEditorAttribute* myAttr = dynamic_cast<caf::PdmUiFilePathEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->m_selectSaveFileName = true;
        }
    }
}
