/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RifEclipseSummaryAddress.h"
#include "RiuQwtSymbol.h"
#include "RiuQwtPlotCurve.h"

#include "cafPdmField.h"
#include "cafPdmFieldCvfColor.h"    
#include "cafPdmObject.h"

#include <QPointer>

class QwtPlot;
class QwtPlotCurve;

//==================================================================================================
///  
///  
//==================================================================================================
class RimPlotCurve : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    typedef caf::AppEnum<RiuQwtPlotCurve::CurveInterpolationEnum> CurveInterpolation;
    typedef caf::AppEnum<RiuQwtPlotCurve::LineStyleEnum> LineStyle;
    typedef caf::AppEnum<RiuQwtSymbol::PointSymbolEnum> PointSymbol;

public:
    RimPlotCurve();
    ~RimPlotCurve() override;

    void                            loadDataAndUpdate(bool updateParentPlot);

    virtual bool                    xValueRange(double* minimumValue, double* maximumValue) const;
    virtual bool                    yValueRange(double* minimumValue, double* maximumValue) const;

    void                            setParentQwtPlotAndReplot(QwtPlot* plot);
    void                            setParentQwtPlotNoReplot(QwtPlot* plot);
    void                            detachQwtCurve();
    void                            reattachQwtCurve();
    QwtPlotCurve*                   qwtPlotCurve() const;

    void                            setColor(const cvf::Color3f& color);
    cvf::Color3f                    color() const { return m_curveColor; }
    void                            setLineStyle(RiuQwtPlotCurve::LineStyleEnum lineStyle);
    void                            setSymbol(RiuQwtSymbol::PointSymbolEnum symbolStyle);
    RiuQwtSymbol::PointSymbolEnum   symbol();
    void                            setSymbolSkipDistance(float distance);
    void                            setSymbolLabel(const QString& label);
    void                            setSymbolSize(int sizeInPixels);
    void                            setLineThickness(int thickness);
    void                            resetAppearance();

    bool                            isCurveVisible() const;
    void                            setCurveVisiblity(bool visible);

    void                            updateCurveNameAndUpdatePlotLegendAndTitle();
    void                            updateCurveNameNoLegendUpdate();

    QString                         curveName() const { return m_curveName; }
    virtual QString                 curveExportDescription(const RifEclipseSummaryAddress& address = RifEclipseSummaryAddress()) const { return m_curveName; }
    void                            setCustomName(const QString& customName);
    void                            updateCurveVisibility(bool updateParentPlot);
    void                            updateLegendEntryVisibilityAndPlotLegend();
    void                            updateLegendEntryVisibilityNoPlotUpdate();

    void                            showLegend(bool show);

    void                            setZOrder(double z);

    virtual void                    updateCurveAppearance();
    bool                            isCrossPlotCurve() const;

protected:

    virtual QString                 createCurveAutoName() = 0;
    virtual void                    updateZoomInParentPlot() = 0;
    virtual void                    onLoadDataAndUpdate(bool updateParentPlot) = 0;
    void                    initAfterRead() override;
    void                            updateCurvePresentation(bool updatePlotLegendAndTitle);

    void                            updateOptionSensitivity();
    void                            updatePlotTitle();
    virtual void                    updateLegendsInPlot();
protected:

    // Overridden PDM methods
    void                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    caf::PdmFieldHandle*    objectToggleField() override;
    caf::PdmFieldHandle*    userDescriptionField() override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly) override;
    void                            appearanceUiOrdering(caf::PdmUiOrdering& uiOrdering);
    void                            curveNameUiOrdering(caf::PdmUiOrdering& uiOrdering);

protected:
    QPointer<QwtPlot>                 m_parentQwtPlot;
    RiuQwtPlotCurve*                  m_qwtPlotCurve;

    caf::PdmField<bool>               m_showCurve;
    caf::PdmField<QString>            m_curveName;
    caf::PdmField<QString>            m_customCurveName;
    caf::PdmField<bool>               m_showLegend;
    QString                           m_symbolLabel;
    caf::PdmField<int>                m_symbolSize;

    caf::PdmField<bool>               m_isUsingAutoName;
    caf::PdmField<cvf::Color3f>       m_curveColor;
    caf::PdmField<int>                m_curveThickness;
    caf::PdmField<float>              m_symbolSkipPixelDistance;
    caf::PdmField<bool>               m_showErrorBars;

    caf::PdmField<PointSymbol>        m_pointSymbol;
    caf::PdmField<LineStyle>          m_lineStyle;
    caf::PdmField<CurveInterpolation> m_curveInterpolation;
    RiuQwtSymbol::LabelPosition       m_symbolLabelPosition;
};


