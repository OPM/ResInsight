#include "Rim2dEclipseViewCollection.h"

#include "Rim2dEclipseView.h"
#include "RimCase.h"

CAF_PDM_SOURCE_INIT(Rim2dEclipseViewCollection, "Eclipse2dViewCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Rim2dEclipseViewCollection::Rim2dEclipseViewCollection()
{
    CAF_PDM_InitObject("2D Contour Maps", ":/2DMaps16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_2dEclipseViews, "EclipseViews", "Contour Maps", ":/CrossSection16x16.png", "", "");
    m_2dEclipseViews.uiCapability()->setUiTreeHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Rim2dEclipseViewCollection::~Rim2dEclipseViewCollection()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<Rim2dEclipseView*> Rim2dEclipseViewCollection::views()
{
    return m_2dEclipseViews.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim2dEclipseViewCollection::push_back(Rim2dEclipseView* contourMap)
{
    m_2dEclipseViews.push_back(contourMap);
}

