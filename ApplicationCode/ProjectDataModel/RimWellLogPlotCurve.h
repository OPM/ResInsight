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

#include "cafPdmField.h"
#include "cafPdmFieldCvfColor.h"    
#include "cafPdmObject.h"

#include "RigWellLogCurveData.h"

#include <vector>

class RigWellLogCurveData;
class RiuWellLogTrackPlot;
class RiuWellLogPlotCurve;
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

    void    setColor(const cvf::Color3f& color);

    bool    depthRange(double* minimumDepth, double* maximumDepth) const;
    bool    valueRange(double* minimumValue, double* maximumValue) const;
    
    void    setPlot(RiuWellLogTrackPlot* plot);
    void    detachCurve();

    bool    isCurveVisibile();

    QwtPlotCurve*               plotCurve() const;
    const RigWellLogCurveData*  curveData() const;
    
    QString         name() const { return m_curveName; }
    void            updateCurveName();
    void            updatePlotTitle();

    virtual QString wellLogChannelName() const = 0;
    virtual void    updatePlotData() = 0;

protected:
    virtual QString createCurveName() = 0;

    void updatePlotConfiguration();
    void updateCurveVisibility();
    void updateTrackAndPlotFromCurveData();
    void updateOptionSensitivity();

    // Overridden PDM methods
    virtual void                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual caf::PdmFieldHandle*    objectToggleField();
    virtual caf::PdmFieldHandle*    userDescriptionField();
    virtual void                    initAfterRead();


    RiuWellLogTrackPlot*            m_plot;
    RiuWellLogPlotCurve*            m_plotCurve;
    cvf::ref<RigWellLogCurveData>   m_curveData;

    caf::PdmField<bool>         m_showCurve;
    caf::PdmField<QString>      m_curveName;
    caf::PdmField<QString>      m_customCurveName;

    caf::PdmField<bool>         m_autoName;
    caf::PdmField<cvf::Color3f> m_curveColor;
};
