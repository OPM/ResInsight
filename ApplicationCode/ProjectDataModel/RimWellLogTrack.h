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

#include <vector>

class RimWellLogCurve;
class RiuWellLogTrack;
class RimWellFlowRateCurve;

class QwtPlotCurve;

//==================================================================================================
///  
///  
//==================================================================================================
class RimWellLogTrack : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimWellLogTrack();
    virtual ~RimWellLogTrack();

    void setDescription(const QString& description);
    bool isVisible();
    void addCurve(RimWellLogCurve* curve);
    void insertCurve(RimWellLogCurve* curve, size_t index);
    void removeCurve(RimWellLogCurve* curve);
    size_t curveIndex(RimWellLogCurve* curve);
    size_t curveCount() { return curves.size(); }
    void setXAxisTitle(const QString& text);

    void recreateViewer();
    void detachAllCurves();

    void loadDataAndUpdate();

    void availableDepthRange(double* minimumDepth, double* maximumDepth);
    void updateXZoomAndParentPlotDepthZoom();
    void updateXZoom();

    RiuWellLogTrack* viewer();
    
    RimWellLogCurve* curveDefinitionFromCurve(const QwtPlotCurve* curve) const;

    void setLogarithmicScale(bool enable);

    std::vector<RimWellFlowRateCurve*> visibleStackedCurves();

    QString description();
    std::vector<RimWellLogCurve* > curvesVector();


protected:
    // Overridden PDM methods
    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);

    void computeAndSetXRangeMinForLogarithmicScale();

    virtual caf::PdmFieldHandle* objectToggleField();
    virtual caf::PdmFieldHandle* userDescriptionField();
    virtual void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering);

private:
    void updateAxisScaleEngine();

private:
    QString m_xAxisTitle;

    // Fields

    caf::PdmField<bool> m_show;
    caf::PdmField<QString> m_userName;
    caf::PdmChildArrayField<RimWellLogCurve*> curves;
    caf::PdmField<double> m_visibleXRangeMin;
    caf::PdmField<double> m_visibleXRangeMax;
    caf::PdmField<bool>   m_isAutoScaleXEnabled;
    caf::PdmField<bool>   m_isLogarithmicScaleEnabled;

    QPointer<RiuWellLogTrack> m_wellLogTrackPlotWidget;
};
