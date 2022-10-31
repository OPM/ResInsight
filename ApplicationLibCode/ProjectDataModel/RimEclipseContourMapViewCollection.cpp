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

    CAF_PDM_InitFieldNoDefault( &m_contourMapViews, "EclipseViews", "Contour Maps", ":/CrossSection16x16.png" );
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
    return m_contourMapViews.children();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapViewCollection::push_back( RimEclipseContourMapView* contourMap )
{
    m_contourMapViews.push_back( contourMap );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapViewCollection::onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                                                         std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    RimEclipseCase* eclipseCase = nullptr;
    this->firstAncestorOrThisOfType( eclipseCase );
    if ( eclipseCase ) eclipseCase->updateConnectedEditors();
}
