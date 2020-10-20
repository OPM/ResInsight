#include "RicExportContourMapToTextUi.h"

#include "cafPdmUiFilePathEditor.h"

CAF_PDM_SOURCE_INIT( RicExportContourMapToTextUi, "RicExportContourMapToTextUi" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicExportContourMapToTextUi::RicExportContourMapToTextUi()
{
    CAF_PDM_InitObject( "Export Contour Map to Text", "", "", "" );

    CAF_PDM_InitField( &m_exportFileName, "ExportFileName", QString(), "Export File Name", "", "", "" );
    m_exportFileName.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_exportLocalCoordinates, "ExportLocalCoordinates", false, "Export Local Coordinates", "", "", "" );
    CAF_PDM_InitField( &m_undefinedValueLabel, "UndefinedValueLabel", QString( "NaN" ), "Undefined Value Label", "", "", "" );
    CAF_PDM_InitField( &m_excludeUndefinedValues, "ExcludeUndefinedValues", false, "Exclude Undefined Values", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicExportContourMapToTextUi::exportFileName() const
{
    return m_exportFileName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportContourMapToTextUi::setExportFileName( const QString& exportFileName )
{
    m_exportFileName = exportFileName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportContourMapToTextUi::exportLocalCoordinates() const
{
    return m_exportLocalCoordinates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicExportContourMapToTextUi::undefinedValueLabel() const
{
    return m_undefinedValueLabel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportContourMapToTextUi::excludeUndefinedValues() const
{
    return m_excludeUndefinedValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportContourMapToTextUi::defineEditorAttribute( const caf::PdmFieldHandle* field,
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
