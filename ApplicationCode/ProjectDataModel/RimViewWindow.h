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

#pragma once

#include "cafPdmObject.h"
#include "cafPdmField.h"
#include "cafPdmChildField.h"

#include <vector>

class RimMdiWindowController;

struct RimMdiWindowGeometry 
{
    RimMdiWindowGeometry() : mainWindowID(-1), x(0), y(0), width(-1), height(-1), isMaximized(false) {}
    bool isValid() const { return (mainWindowID >= 0 && width >= 0 && height >= 0);}

    int mainWindowID;

    int x; 
    int y; 
    int width;
    int height; 
    bool isMaximized;
};

class RimViewWindow : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimViewWindow(void);
    virtual ~RimViewWindow(void);

    void                         loadDataAndUpdate();
    void                         handleMdiWindowClosed();
    void                         updateMdiWindowVisibility(); 
                                 
    void                         setAs3DViewMdiWindow()  { setAsMdiWindow(0); }
    void                         setAsPlotMdiWindow()    { setAsMdiWindow(1); }
    bool                         isMdiWindow() const;
                                 
    void                         setMdiWindowGeometry(const RimMdiWindowGeometry& windowGeometry);
    RimMdiWindowGeometry         mdiWindowGeometry();
                                 
    virtual QWidget*             viewWidget() = 0;
                                 
    virtual QImage               snapshotWindowContent() = 0;
    virtual void                 zoomAll() = 0;

protected:
    void                         removeMdiWindowFromMdiArea(); 

    ///////// Interface for the Window controller
    friend class RimMdiWindowController;

    virtual QWidget*             createViewWidget(QWidget* mainWindowParent) = 0; 
    virtual void                 updateViewWidgetAfterCreation() {};
    virtual void                 updateMdiWindowTitle(); // Has real default implementation
    virtual void                 deleteViewWidget() = 0;
    virtual void                 onLoadDataAndUpdate() = 0; 
    virtual bool                 isWindowVisible() { return m_showWindow();} // Virtual To allow special visibility control
    //////////

    // Derived classes are not supposed to override this function. The intention is to always use m_showWindow
    // as the objectToggleField for this class. This way the visibility of a widget being part of a composite widget
    // can be controlled from the project tree using check box toggles
    virtual caf::PdmFieldHandle* objectToggleField() override final;
    virtual void                 fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual void                 initAfterRead() override;
                                 
    caf::PdmField<bool>          m_showWindow;

private:                         
    void                         setAsMdiWindow(int mainWindowID);

    caf::PdmChildField<RimMdiWindowController*> m_windowController;


    // Obsoleted field
    caf::PdmField< std::vector<int> > obsoleteField_windowGeometry;
};

