#include "RimEclipseContourMapViewCollection.h"

#include "RimCase.h"
#include "RimEclipseContourMapView.h"

CAF_PDM_SOURCE_INIT( RimEclipseContourMapViewCollection, "Eclipse2dViewCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseContourMapViewCollection::RimEclipseContourMapViewCollection()
{
    CAF_PDM_InitObject( "Contour Maps", ":/2DMaps16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_contourMapViews, "EclipseViews", "Contour Maps", ":/CrossSection16x16.png", "", "" );
    m_contourMapViews.uiCapability()->setUiTreeHidden( true );
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
    return m_contourMapViews.childObjectsByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapViewCollection::push_back( RimEclipseContourMapView* contourMap )
{
    m_contourMapViews.push_back( contourMap );
}
