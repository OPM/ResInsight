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

#include <vector>

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

    QwtPlotCurve*   plotCurve() const;
    
    void            updatePlotTitle();

    virtual void    updatePlotData() = 0;

protected:
    virtual QString createCurveName() = 0;

    void updatePlotConfiguration();
    void updateCurveVisibility();
    void updateOptionSensitivity();
    void updateTrackAndPlotFromCurveData();

    // Overridden PDM methods
    virtual void                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual caf::PdmFieldHandle*    objectToggleField();
    virtual caf::PdmFieldHandle*    userDescriptionField();
    virtual void                    defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering);
    virtual void                    initAfterRead();


    RiuWellLogTrackPlot*    m_plot;
    RiuWellLogPlotCurve*    m_plotCurve;

    caf::PdmField<bool>         m_showCurve;
    caf::PdmField<QString>      m_customCurveName;
    caf::PdmField<QString>      m_generatedCurveName;
    caf::PdmField<bool>         m_useCustomCurveName;
    caf::PdmField<cvf::Color3f> m_curveColor;
};
