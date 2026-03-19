#include "ColorTriplet.h"

#include "cafPdmUiColorEditor.h"

CAF_PDM_SOURCE_INIT( ColorTriplet, "ColorTriplet" );

ColorTriplet::ColorTriplet()
{
    CAF_PDM_InitObject( "ColorTriplet", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_colorField, "Color", "color", "", "", "" );
    m_colorField.uiCapability()->setUiEditorTypeName( caf::PdmUiColorEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_categoryValue, "category", "category value", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_categoryText, "text", "category text", "", "", "" );
}
