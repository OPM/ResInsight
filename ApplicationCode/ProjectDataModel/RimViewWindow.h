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
    RimMdiWindowGeometry() : mainWindowID(-1), x(0), y(0), width(-1), height(-1)  {}
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

    void                 removeWidgetFromMDI(); 
    void                 handleViewerDeletion();
    void                 updateViewerWidgetBasic(); 

    void                 setAs3DMDI()   { setAsMDI(0); }
    void                 setAsPlotMDI() { setAsMDI(1); }

    void                 setMdiWindowGeometry(const RimMdiWindowGeometry& windowGeometry);
    RimMdiWindowGeometry mdiWindowGeometry();

    virtual QImage       snapshotWindowContent() = 0;
    virtual QWidget*     viewWidget() = 0;

    virtual void         zoomAll() = 0;

protected:
    virtual void         fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;


    ///////// Interface for the Window controller
    friend class RimMdiWindowController;

    virtual caf::PdmField<bool>* getShowWindowField() = 0; 
    virtual QWidget*             createViewWidget(QWidget* mainWindowParent) = 0; 
    virtual void                 updateViewWidgetAfterCreation() {};
    virtual void                 updateViewerWidgetWindowTitle(); // Has real default implementation
    virtual void                 deleteViewWidget() = 0; 


    //////////

    void                 setAsMDI(int mainWindowID);

    caf::PdmChildField<RimMdiWindowController*> m_windowController;

    // Obsoleted field
    virtual void initAfterRead() override;
    caf::PdmField< std::vector<int> > m_windowGeometry;


};

