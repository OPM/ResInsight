#include "RicfCreateView.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "Rim3dView.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechView.h"
#include "RimProject.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_PDM_SOURCE_INIT( RicfCreateViewResult, "createViewResult" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfCreateViewResult::RicfCreateViewResult( int viewId /*= -1*/ )
{
    CAF_PDM_InitObject( "view_result", "", "", "" );
    CAF_PDM_InitField( &this->viewId, "viewId", viewId, "", "", "", "" );
}

CAF_PDM_SOURCE_INIT( RicfCreateView, "createView" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfCreateView::RicfCreateView()
{
    RICF_InitField( &m_caseId, "caseId", -1, "Case Id", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfCommandResponse RicfCreateView::execute()
{
    RimProject*           project = RiaApplication::instance()->project();
    std::vector<RimCase*> allCases;
    project->allCases( allCases );

    for ( RimCase* rimCase : allCases )
    {
        if ( rimCase->caseId == m_caseId() )
        {
            int             viewId      = -1;
            RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( rimCase );
            RimGeoMechCase* geoMechCase = dynamic_cast<RimGeoMechCase*>( rimCase );
            if ( eclipseCase )
            {
                RimEclipseView* view = eclipseCase->createAndAddReservoirView();
                viewId               = view->id();
                view->loadDataAndUpdate();
                eclipseCase->updateConnectedEditors();
                Riu3DMainWindowTools::setExpanded( view );
            }
            else if ( geoMechCase )
            {
                RimGeoMechView* view = geoMechCase->createAndAddReservoirView();
                viewId               = view->id();
                view->loadDataAndUpdate();
                geoMechCase->updateConnectedEditors();
                Riu3DMainWindowTools::setExpanded( view );
            }

            if ( viewId >= 0 )
            {
                RicfCommandResponse response;
                response.setResult( new RicfCreateViewResult( viewId ) );
                return response;
            }
        }
    }

    QString error = QString( "createView: Could not create view for case id %1" ).arg( m_caseId() );
    RiaLogging::error( error );
    return RicfCommandResponse( RicfCommandResponse::COMMAND_ERROR, error );
}
