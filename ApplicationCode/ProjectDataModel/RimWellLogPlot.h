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

    void    addTrace(RimWellLogPlotTrace* trace);
    size_t  traceCount() { return traces.size();}

    void loadDataAndUpdate();

    RiuWellLogPlot* viewer();

    void zoomDepth(double zoomFactor);
    void panDepth(double panFactor);
    void setDepthRange(double minimumDepth, double maximumDepth);

    void updateAvailableDepthRange();
    bool availableDepthRange(double* minimumDepth, double* maximumDepth);

    void visibleDepthRange(double* minimumDepth, double* maximumDepth) const;
    void updateAxisRanges();
    void setVisibleDepthRangeFromContents();

protected:

    // Overridden PDM methods
    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual void setupBeforeSave();

private:
    void updateViewerWidget();
    void recreateTracePlots();
    void detachAllCurves();

    virtual caf::PdmFieldHandle* objectToggleField();



private:
    QPointer<RiuWellLogPlot> m_viewer;
    
    caf::PdmField<bool> showWindow;
    caf::PdmField< std::vector<int> >       windowGeometry;

    caf::PdmChildArrayField<RimWellLogPlotTrace*> traces;
    
    caf::PdmField<double> m_minimumVisibleDepth;
    caf::PdmField<double> m_maximumVisibleDepth;

    double m_depthRangeMinimum;
    double m_depthRangeMaximum;
};
