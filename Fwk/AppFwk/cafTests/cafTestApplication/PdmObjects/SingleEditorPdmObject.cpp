#include "SingleEditorPdmObject.h"

#include "cafPdmUiOrdering.h"

CAF_PDM_SOURCE_INIT( SingleEditorPdmObject, "SingleEditorObject" );

SingleEditorPdmObject::SingleEditorPdmObject()
{
    CAF_PDM_InitObject( "Single Editor Object",
                        "",
                        "This object is a demo of the CAF framework",
                        "This object is a demo of the CAF framework" );

    CAF_PDM_InitField( &m_intFieldStandard,
                       "Standard",
                       0,
                       "Fairly Wide Label",
                       "",
                       "Enter some small number here",
                       "This is a place you can enter a small integer value if you want" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void SingleEditorPdmObject::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_intFieldStandard );
}
