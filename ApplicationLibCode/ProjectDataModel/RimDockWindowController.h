/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

class RiuMainWindowBase;
class RimViewWindow;
struct RimMdiWindowGeometry;

//==================================================================================================
///
///
//==================================================================================================
class RimDockWindowController : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimDockWindowController();
    ~RimDockWindowController() override;

    void setMainWindowId( int mainId );
    void setViewToControl( RimViewWindow* view );
    int  mainWindowId() const;

    void updateViewerWidget();
    void handleViewerDeletion();
    void removeWindowFromDock();

protected:
    RimViewWindow*     viewPdmObject();
    QWidget*           viewWidget();
    RiuMainWindowBase* getMainWindow();

    // Overridden PDM methods
    void setupBeforeSave() override;

private:
    caf::PdmField<int>               m_mainWindowID;
    caf::PdmPtrField<RimViewWindow*> m_viewToControl;
};
