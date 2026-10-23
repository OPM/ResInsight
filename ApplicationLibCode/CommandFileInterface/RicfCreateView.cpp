#include "RicfCreateView.h"

#include "RicfCommandForwarding.h"

#include "Rim3dView.h"
#include "RimCase.h"
#include "RimcCase.h"

#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RicfCreateViewResult, "createViewResult" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfCreateViewResult::RicfCreateViewResult( int viewId /*= -1*/ )
{
    CAF_PDM_InitObject( "view_result" );
    CAF_PDM_InitField( &this->viewId, "viewId", viewId, "" );
}

CAF_PDM_SOURCE_INIT( RicfCreateView, "createView" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfCreateView::RicfCreateView()
{
    CAF_PDM_InitScriptableField( &m_caseId, "caseId", -1, "Case Id" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfCreateView::execute()
{
    const QString commandName = classKeyword();

    auto rimCase = RicfForwarding::findCase( m_caseId() );
    if ( !rimCase ) return RicfForwarding::errorResponse( rimCase.error(), commandName );

    RimCase_createView method( rimCase.value() );

    auto result = method.execute();
    if ( !result ) return RicfForwarding::errorResponse( result.error(), commandName );

    auto* view = dynamic_cast<Rim3dView*>( result.value() );
    if ( !view ) return RicfForwarding::errorResponse( "Created object is not a view", commandName );

    caf::PdmScriptResponse response;
    response.setResult( new RicfCreateViewResult( view->id() ) );
    return response;
}
