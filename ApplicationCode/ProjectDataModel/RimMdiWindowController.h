/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#pragma once

#include "cafPdmObject.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"

#if 0
class RimMdiWindowController;

class RimViewWindow : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimViewWindow(void);
    virtual ~RimViewWindow(void)
    {
        if (m_windowController()) delete m_windowController();
    }

    void                 handleViewerDeletion()    { if (m_windowController()) m_windowController->handleViewerDeletion();  }
    void                 updateViewerWidgetBasic() { if (m_windowController()) m_windowController->updateViewerWidget(); } 

    void                 setAs3DMDI()   { setAsMDI(0); }
    void                 setAsPlotMDI() { setAsMDI(1); }

    void                 setMdiWindowGeometry(const RimMdiWindowGeometry& windowGeometry);
    RimMdiWindowGeometry mdiWindowGeometry();

    virtual QImage       snapshotWindowContent() = 0;
    virtual void         zoomAll() = 0;
                         
protected:
    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override
    {
        if ( m_windowController() )
        {
            if ( changedField  == showWindowField() )
            {
                m_windowController()->showWindowFieldChangedByUi();
            }
        }
    }

private:

    ///////// Interface for the Window controller
    friend class RimMdiWindowController;

    virtual QWidget*     viewWidget() = 0;
    virtual QWidget*     createViewWidget(QWidget* mainWindowParent) = 0;
    virtual void         updateViewWidgetAfterCreation() = 0;
    virtual void         deleteViewWidget() = 0;

    virtual caf::PdmField<bool>* showWindowField() = 0; 

    //////////

    void                 setAsMDI(int mainWindowID)
    {
        if (!m_windowController())
        {
            m_windowController = new RimMdiWindowController;
            RimMdiWindowGeometry mwg;
            mwg.mainWindowID = mainWindowID;
            setMdiWindowGeometry(mwg);
        }
    }

    caf::PdmChildField<RimMdiWindowController*> m_windowController;

    // Obsolete field
    caf::PdmField< std::vector<int> > m_windowGeometry;
};
#endif

class RimViewWindow;
class RiuMainWindowBase;
struct RimMdiWindowGeometry;

//==================================================================================================
///  
///  
//==================================================================================================
class RimMdiWindowController : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimMdiWindowController();
    virtual ~RimMdiWindowController();

    void                      setMdiWindowGeometry(const RimMdiWindowGeometry& windowGeometry);
    RimMdiWindowGeometry      mdiWindowGeometry();

    void                      updateViewerWidget();
    void                      handleViewerDeletion();
    void                      removeWindowFromMDI();
    void                      showWindowFieldChangedByUi();

protected:

    RimViewWindow*            viewPdmObject();
    caf::PdmField<bool>&      showWindowField();
    QWidget*                  viewWidget();
    RiuMainWindowBase*        getMainWindow();
 
    // Overridden PDM methods
    virtual void              setupBeforeSave() override;

private:
    
    caf::PdmField< int >      m_mainWindowID;
     
    caf::PdmField< int >      m_x; 
    caf::PdmField< int >      m_y; 
    caf::PdmField< int >      m_width;
    caf::PdmField< int >      m_height; 
    caf::PdmField< bool>      m_isMaximized;
};


