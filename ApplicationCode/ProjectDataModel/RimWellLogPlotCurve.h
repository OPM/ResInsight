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
#include "cafPdmFieldCvfColor.h"    

#include <vector>

class RiuWellLogTracePlot;
class QwtPlotCurve;
class QString;

//==================================================================================================
///  
///  
//==================================================================================================
class RimWellLogPlotCurve : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimWellLogPlotCurve();
    virtual ~RimWellLogPlotCurve();

    void setDescription(QString description) {m_userName = description;}
    void    setColor(const cvf::Color3f& color);

    bool    depthRange(double* minimumDepth, double* maximumDepth) const;
    
    void    setPlot(RiuWellLogTracePlot* plot);
    void    detachCurve();

    QwtPlotCurve* plotCurve() const { return m_plotCurve; }
    
    virtual void                 updatePlotData();

protected:
    void updateCurveVisibility();

    // Overridden PDM methods
    virtual void                 fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual caf::PdmFieldHandle* objectToggleField();
    virtual caf::PdmFieldHandle* userDescriptionField();


    caf::PdmField<bool>         m_showCurve;
    caf::PdmField<QString>      m_userName;
    caf::PdmField<cvf::Color3f> m_curveColor;
    // caf::PdmField<Linestyle> m_lineStyle;
    // caf::PdmField<int>       m_lineWidth;

    RiuWellLogTracePlot*    m_plot;
    QwtPlotCurve*           m_plotCurve;
};
