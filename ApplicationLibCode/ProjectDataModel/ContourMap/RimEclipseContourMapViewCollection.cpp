#include "RimEclipseContourMapViewCollection.h"

#include "RimEclipseCase.h"
#include "RimEclipseContourMapView.h"

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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapViewCollection::onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                                                         std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    auto eclipseCase = firstAncestorOrThisOfType<RimEclipseCase>();
    if ( eclipseCase ) eclipseCase->updateConnectedEditors();
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
