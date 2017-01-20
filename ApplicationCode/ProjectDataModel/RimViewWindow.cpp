/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
// 
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
// 
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RimViewWindow.h"
#include "RimMdiWindowController.h"
#include "cvfAssert.h"
#include <QWidget>

CAF_PDM_XML_ABSTRACT_SOURCE_INIT(RimViewWindow, "ViewWindow"); // Do not use. Abstract class 

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimViewWindow::RimViewWindow(void)
{
    CAF_PDM_InitFieldNoDefault(&m_windowController, "WindowController", "", "", "", "");
    m_windowController.uiCapability()->setUiHidden(true);
    m_windowController.uiCapability()->setUiTreeChildrenHidden(true);

    // Obsolete field
    CAF_PDM_InitFieldNoDefault(&obsoleteField_windowGeometry, "WindowGeometry", "", "", "", "");
    obsoleteField_windowGeometry.uiCapability()->setUiHidden(true);
    obsoleteField_windowGeometry.xmlCapability()->setIOWritable(false);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimViewWindow::~RimViewWindow(void)
{
    if ( m_windowController() ) delete  m_windowController() ;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewWindow::removeMdiWindowFromMdiArea()
{
    if ( m_windowController() ) m_windowController->removeWindowFromMDI();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewWindow::handleMdiWindowClosed()
{
    if ( m_windowController() ) m_windowController->handleViewerDeletion();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewWindow::updateMdiWindowVisibility()
{
    if ( m_windowController() ) m_windowController->updateViewerWidget();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewWindow::setMdiWindowGeometry(const RimMdiWindowGeometry& windowGeometry)
{
    CVF_ASSERT(m_windowController());

    if (m_windowController()) m_windowController()->setMdiWindowGeometry(windowGeometry);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimMdiWindowGeometry RimViewWindow::mdiWindowGeometry()
{
    CVF_ASSERT(m_windowController());

    if (m_windowController()) return m_windowController()->mdiWindowGeometry();
    else return RimMdiWindowGeometry();

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewWindow::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if ( m_windowController() )
    {
        if ( changedField  == getShowWindowField() )
        {
            m_windowController()->showWindowFieldChangedByUi();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewWindow::updateMdiWindowTitle()
{
    if ( viewWidget() )
    {
        caf::PdmUiFieldHandle* uiFieldHandle = this->userDescriptionField()->uiCapability();
        if ( uiFieldHandle )
        {
            QVariant v = uiFieldHandle->uiValue();
            viewWidget()->setWindowTitle(v.toString());
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewWindow::setAsMdiWindow(int mainWindowID)
{
    if ( !m_windowController() )
    {
        m_windowController = new RimMdiWindowController;
        RimMdiWindowGeometry mwg;
        mwg.mainWindowID = mainWindowID;
        setMdiWindowGeometry(mwg);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#include "RimView.h"

void RimViewWindow::initAfterRead()
{
   if (obsoleteField_windowGeometry.value().size() == 5)
   {
       RimMdiWindowGeometry wg;
       int mainWindowID = -1;
       
       if (dynamic_cast<RimView*> (this))
          mainWindowID = 0;
       else 
          mainWindowID = 1;

       wg.mainWindowID = mainWindowID; 
       wg.x = obsoleteField_windowGeometry.value()[0];
       wg.y = obsoleteField_windowGeometry.value()[1];
       wg.width = obsoleteField_windowGeometry.value()[2];
       wg.height = obsoleteField_windowGeometry.value()[3];
       wg.isMaximized = obsoleteField_windowGeometry.value()[4];

       setAsMdiWindow(mainWindowID);
       setMdiWindowGeometry(wg);
   }
}

