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
#include <QPointer>
#include <vector>

class RigWellLogCurveData;
class RiuWellLogTrack;
class RiuLineSegmentQwtPlotCurve;
class QwtPlotCurve;
class QString;

//==================================================================================================
///  
///  
//==================================================================================================
class RimWellLogCurve : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimWellLogCurve();
    virtual ~RimWellLogCurve();

    void                            setColor(const cvf::Color3f& color);

    bool                            depthRange(double* minimumDepth, double* maximumDepth) const;
    bool                            valueRange(double* minimumValue, double* maximumValue) const;
    
    void                            setQwtTrack(RiuWellLogTrack* plot);
    void                            detachQwtCurve();

    bool                            isCurveVisible() const;

    QwtPlotCurve*                   plotCurve() const;
    const RigWellLogCurveData*      curveData() const;
    
    QString                         name() const { return m_curveName; }
    void                            updateCurveName();
    void                            updatePlotTitle();

    virtual QString                 wellName() const = 0;
    virtual QString                 wellLogChannelName() const = 0;
    virtual QString                 wellDate() const  { return ""; };
    virtual void                    updatePlotData() = 0;

protected:
    virtual QString                 createCurveName() = 0;

    void                            updatePlotConfiguration();
    void                            updateCurveVisibility();
    void                            zoomAllOwnerTrackAndPlot();
    void                            updateOptionSensitivity();
    void                            updateCurvePen();

    // Overridden PDM methods
    virtual void                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual caf::PdmFieldHandle*    objectToggleField();
    virtual caf::PdmFieldHandle*    userDescriptionField();
    virtual void                    initAfterRead();
    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly);


    QPointer<RiuWellLogTrack>   m_ownerQwtTrack;
    RiuLineSegmentQwtPlotCurve*            m_qwtPlotCurve;
    cvf::ref<RigWellLogCurveData>   m_curveData;

    caf::PdmField<bool>             m_showCurve;
    caf::PdmField<QString>          m_curveName;
    caf::PdmField<QString>          m_customCurveName;

    caf::PdmField<bool>             m_autoName;
    caf::PdmField<cvf::Color3f>     m_curveColor;
    caf::PdmField<float>            m_curveThickness;
};
