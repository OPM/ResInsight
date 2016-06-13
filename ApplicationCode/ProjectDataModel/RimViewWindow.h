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
#include "QPointer"

struct RimMdiWindowGeometry 
{
    RimMdiWindowGeometry() : x(0), y(0), width(-1), height(-1)  {}
    bool isValid() const { return (width >= 0 && height >= 0);}

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

    void                 setMdiWindowGeometry(const RimMdiWindowGeometry& windowGeometry);
    RimMdiWindowGeometry mdiWindowGeometry();

protected:
    void                 setViewWidget(QWidget* viewWidget);

    // Possible abilities of this class

    //virtual QImage snapshotWindowContent() = 0;
    //caf::PdmField<QString>                  name;
    //caf::PdmField<bool>                     showWindow;

private:
    caf::PdmField< std::vector<int> > m_windowGeometry;
    QPointer<QWidget>                 m_viewWidget;
};

