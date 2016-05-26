#include "RimViewWindow.h"
#include "QWidget"

CAF_PDM_XML_ABSTRACT_SOURCE_INIT(RimViewWindow, "ViewWindow"); // Do not use. Abstract class 

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimViewWindow::RimViewWindow(void)
{
    CAF_PDM_InitFieldNoDefault(&m_windowGeometry, "WindowGeometry", "", "", "", "");
    m_windowGeometry.uiCapability()->setUiHidden(true);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimViewWindow::~RimViewWindow(void)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewWindow::setViewWidget(QWidget* viewWidget)
{
    m_viewWidget = viewWidget;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewWindow::setMdiWindowGeometry(const RimMdiWindowGeometry& windowGeometry)
{
    std::vector<int> geom;
    geom.clear();
    if (windowGeometry.isValid())
    {
        geom.push_back(windowGeometry.x);
        geom.push_back(windowGeometry.y);
        geom.push_back(windowGeometry.width);
        geom.push_back(windowGeometry.height);
        geom.push_back(windowGeometry.isMaximized);
    }
    m_windowGeometry.setValue(geom);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimMdiWindowGeometry RimViewWindow::mdiWindowGeometry()
{

    RimMdiWindowGeometry wg;
    if (m_windowGeometry.value().size() == 5)
    {
        wg.x = m_windowGeometry.value()[0];
        wg.y = m_windowGeometry.value()[1];
        wg.width = m_windowGeometry.value()[2];
        wg.height = m_windowGeometry.value()[3];
        wg.isMaximized = m_windowGeometry.value()[4];
    }

    return wg;
}

