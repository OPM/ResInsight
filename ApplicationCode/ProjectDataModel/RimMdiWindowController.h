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

protected:

    RimViewWindow*            viewPdmObject();
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


