#include "RicfCloneView.h"

#include "RicfCommandForwarding.h"
#include "RicfCreateView.h"

#include "Rim3dView.h"
#include "RimcGridView.h"

#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RicfCloneView, "cloneView" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfCloneView::RicfCloneView()
{
    CAF_PDM_InitScriptableField( &m_viewId, "viewId", -1, "View Id" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfCloneView::execute()
{
    const QString commandName = classKeyword();

    auto view = RicfForwarding::findView( m_viewId() );
    if ( !view ) return RicfForwarding::errorResponse( view.error(), commandName );

    Rim3dView_clone method( view.value() );

    auto result = method.execute();
    if ( !result ) return RicfForwarding::errorResponse( result.error(), commandName );

    auto* newView = dynamic_cast<Rim3dView*>( result.value() );
    if ( !newView ) return RicfForwarding::errorResponse( "Cloned object is not a view", commandName );

    caf::PdmScriptResponse response;
    response.setResult( new RicfCreateViewResult( newView->id() ) );
    return response;
}
