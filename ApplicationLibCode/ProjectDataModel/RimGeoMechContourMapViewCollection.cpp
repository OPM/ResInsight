#include "RimGeoMechContourMapViewCollection.h"

#include "RimCase.h"
#include "RimGeoMechContourMapView.h"

CAF_PDM_SOURCE_INIT( RimGeoMechContourMapViewCollection, "GeoMech2dViewCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechContourMapViewCollection::RimGeoMechContourMapViewCollection()
{
    CAF_PDM_InitObject( "GeoMech Contour Maps", ":/2DMaps16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_contourMapViews, "GeoMechViews", "Contour Maps", ":/CrossSection16x16.png", "", "" );
    m_contourMapViews.uiCapability()->setUiTreeHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechContourMapViewCollection::~RimGeoMechContourMapViewCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimGeoMechContourMapView*> RimGeoMechContourMapViewCollection::views()
{
    return m_contourMapViews.childObjectsByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapViewCollection::push_back( RimGeoMechContourMapView* contourMap )
{
    m_contourMapViews.push_back( contourMap );
}
