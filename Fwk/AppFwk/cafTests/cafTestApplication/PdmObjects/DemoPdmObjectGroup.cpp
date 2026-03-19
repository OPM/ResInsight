#include "DemoPdmObjectGroup.h"

CAF_PDM_SOURCE_INIT( DemoPdmObjectGroup, "DemoPdmObjectGroup" );

DemoPdmObjectGroup::DemoPdmObjectGroup()
{
    CAF_PDM_InitFieldNoDefault( &objects, "PdmObjects", "MyRootObject", "", "", "" );

    objects.uiCapability()->setUiHidden( true );
}
