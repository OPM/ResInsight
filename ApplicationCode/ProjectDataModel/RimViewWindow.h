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

class RimViewWindow : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimViewWindow(void);
    virtual ~RimViewWindow(void);


protected:
    void setViewWidget(QWidget* viewWidget);

    // Possible abilities of this class

    //caf::PdmField< std::vector<int> >       windowGeometry;
    //virtual QImage snapshotWindowContent() = 0;
    //caf::PdmField<QString>                  name;
    //caf::PdmField<bool>                     showWindow;
   

private:
    QPointer<QWidget> m_viewWidget;
};

