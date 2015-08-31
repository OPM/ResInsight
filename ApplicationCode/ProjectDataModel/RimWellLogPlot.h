/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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
#include "cafPdmChildArrayField.h"

#include <QPointer>

class RiuWellLogPlot;
class RimWellLogPlotTrace;


//==================================================================================================
///  
///  
//==================================================================================================
class RimWellLogPlot : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimWellLogPlot();
    virtual ~RimWellLogPlot();

    caf::PdmChildArrayField<RimWellLogPlotTrace*> traces;
    caf::PdmField<bool> showWindow;

    void addTrace();

    RiuWellLogPlot* viewer();

protected:

    // Overridden PDM methods
    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);

private:
    void updateViewerWidget();

    virtual caf::PdmFieldHandle* objectToggleField();

private:
    QPointer<RiuWellLogPlot> m_viewer;
    
    caf::PdmField<double> minimumDepth;
    caf::PdmField<double> maximumDepth;
};
