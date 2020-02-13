#include "RicfCloneView.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RicfCreateView.h"

#include "Rim3dView.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechView.h"
#include "RimProject.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_PDM_SOURCE_INIT( RicfCloneView, "cloneView" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfCloneView::RicfCloneView()
{
    RICF_InitField( &m_viewId, "viewId", -1, "View Id", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfCommandResponse RicfCloneView::execute()
{
    RimProject*             project = RiaApplication::instance()->project();
    std::vector<Rim3dView*> allViews;
    project->descendantsIncludingThisOfType( allViews );

    for ( Rim3dView* view : allViews )
    {
        if ( view->id() == m_viewId() )
        {
            const RimEclipseView* eclipseView = dynamic_cast<const RimEclipseView*>( view );
            const RimGeoMechView* geoMechView = dynamic_cast<const RimGeoMechView*>( view );

            int newViewId = -1;
            if ( eclipseView )
            {
                RimEclipseCase* eclipseCase    = eclipseView->eclipseCase();
                RimEclipseView* newEclipseView = eclipseCase->createCopyAndAddView( eclipseView );
                newEclipseView->loadDataAndUpdate();
                newViewId = newEclipseView->id();
                eclipseCase->updateConnectedEditors();
                Riu3DMainWindowTools::setExpanded( newEclipseView );
            }
            else if ( geoMechView )
            {
                RimGeoMechCase* geoMechCase    = geoMechView->geoMechCase();
                RimGeoMechView* newGeoMechView = geoMechCase->createCopyAndAddView( geoMechView );
                newGeoMechView->loadDataAndUpdate();
                newViewId = newGeoMechView->id();
                geoMechCase->updateConnectedEditors();
                Riu3DMainWindowTools::setExpanded( view );
            }

            if ( newViewId >= 0 )
            {
                RicfCommandResponse response;
                response.setResult( new RicfCreateViewResult( newViewId ) );
                return response;
            }
        }
    }

    QString error = QString( "cloneView: Could not clone view with id %1" ).arg( m_viewId() );
    RiaLogging::error( error );
    return RicfCommandResponse( RicfCommandResponse::COMMAND_ERROR, error );
}
