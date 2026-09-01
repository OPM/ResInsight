#include "RimEclipseContourMapViewCollection.h"

#include "ContourMap/RimContourMapInViewCollection.h"
#include "Rim3dView.h"
#include "RimEclipseCase.h"
#include "RimEclipseContourMapView.h"
#include "RimProject.h"

CAF_PDM_SOURCE_INIT( RimEclipseContourMapViewCollection, "Eclipse2dViewCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseContourMapViewCollection::RimEclipseContourMapViewCollection()
{
    CAF_PDM_InitObject( "Contour Maps", ":/2DMaps16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_items, "EclipseViews", "Contour Maps", ":/CrossSection16x16.png" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseContourMapViewCollection::~RimEclipseContourMapViewCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipseContourMapView*> RimEclipseContourMapViewCollection::views()
{
    return items();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapViewCollection::addView( RimEclipseContourMapView* contourMap )
{
    addItem( contourMap );

    RimContourMapInViewCollection::updateViewTreeItemsInAllViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapViewCollection::onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                                                         std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    auto eclipseCase = firstAncestorOrThisOfType<RimEclipseCase>();
    if ( eclipseCase ) eclipseCase->updateConnectedEditors();

    RimContourMapInViewCollection::updateViewTreeItemsInAllViews();

    // A deleted contour map may have been visible in a 3d view
    if ( RimProject* project = RimProject::current() )
    {
        for ( Rim3dView* view : project->allViews() )
        {
            if ( view ) view->scheduleCreateDisplayModelAndRedraw();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapViewCollection::clearWithoutDelete()
{
    m_items.clearWithoutDelete();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapViewCollection::removeChild( RimEclipseContourMapView* contourMap )
{
    m_items.removeChild( contourMap );
}
