#include "RimGeoMechContourMapViewCollection.h"

#include "RimCase.h"

CAF_PDM_SOURCE_INIT( RimGeoMechContourMapViewCollection, "GeoMech2dViewCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechContourMapViewCollection::RimGeoMechContourMapViewCollection()
{
    CAF_PDM_InitObject( "GeoMech Contour Maps", ":/2DMaps16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_items, "GeoMechViews", "Contour Maps", ":/CrossSection16x16.png" );
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
    return items();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapViewCollection::addView( RimGeoMechContourMapView* contourMap )
{
    addItem( contourMap );
}
