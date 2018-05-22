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

#include "cafPdmField.h"
#include "cafPdmFieldCvfColor.h"    
#include "cafPdmObject.h"

#include <QPointer>

class RiuLineSegmentQwtPlotCurve;

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
    enum LineStyleEnum
    {
        STYLE_NONE,
        STYLE_SOLID,
        STYLE_DASH,
        STYLE_DOT,
        STYLE_DASH_DOT
    };

    enum PointSymbolEnum
    {
        SYMBOL_NONE,
        SYMBOL_ELLIPSE,
        SYMBOL_RECT,
        SYMBOL_DIAMOND,
        SYMBOL_TRIANGLE,
        SYMBOL_CROSS,
        SYMBOL_XCROSS
    };

    enum CurveInterpolationEnum
    {
        INTERPOLATION_POINT_TO_POINT,
        INTERPOLATION_STEP_LEFT,
    };

    typedef caf::AppEnum<CurveInterpolationEnum> CurveInterpolation;

public:
    RimPlotCurve();
    virtual ~RimPlotCurve();

    void                            loadDataAndUpdate(bool updateParentPlot);

    void                            setParentQwtPlotAndReplot(QwtPlot* plot);
    void                            setParentQwtPlotNoReplot(QwtPlot* plot);
    void                            detachQwtCurve();
    QwtPlotCurve*                   qwtPlotCurve() const;

    void                            setColor(const cvf::Color3f& color);
    cvf::Color3f                    color() const { return m_curveColor; }
    void                            setLineStyle(LineStyleEnum lineStyle);
    void                            setSymbol(PointSymbolEnum symbolStyle);
    PointSymbolEnum                 symbol();
    void                            setSymbolSkipDinstance(float distance);
    void                            setLineThickness(int thickness);
    void                            resetAppearance();

    bool                            isCurveVisible() const;
    void                            setCurveVisiblity(bool visible);

    void                            updateCurveNameAndUpdatePlotLegend();
    void                            updateCurveNameNoLegendUpdate();

    QString                         curveName() const { return m_curveName; }

    void                            updateCurveVisibility(bool updateParentPlot);
    void                            updateLegendEntryVisibilityAndPlotLegend();
    void                            updateLegendEntryVisibilityNoPlotUpdate();

    void                            showLegend(bool show);

    void                            setZOrder(double z);

    virtual void                    updateCurveAppearance();

protected:

    virtual QString                 createCurveAutoName() = 0;
    virtual void                    updateZoomInParentPlot() = 0;
    virtual void                    onLoadDataAndUpdate(bool updateParentPlot) = 0;

    void                            updateCurvePresentation(bool updatePlotLegend);

    void                            updateOptionSensitivity();

protected:

    // Overridden PDM methods
    virtual void                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual caf::PdmFieldHandle*    objectToggleField();
    virtual caf::PdmFieldHandle*    userDescriptionField();
    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly);
    void                            appearanceUiOrdering(caf::PdmUiOrdering& uiOrdering);
    void                            curveNameUiOrdering(caf::PdmUiOrdering& uiOrdering);

protected:
    QPointer<QwtPlot>               m_parentQwtPlot;
    RiuLineSegmentQwtPlotCurve*     m_qwtPlotCurve;

    caf::PdmField<bool>             m_showCurve;
    caf::PdmField<QString>          m_curveName;
    caf::PdmField<QString>          m_customCurveName;
    caf::PdmField<bool>             m_showLegend;

    caf::PdmField<bool>             m_isUsingAutoName;
    caf::PdmField<cvf::Color3f>     m_curveColor;
    caf::PdmField<int>              m_curveThickness;
    caf::PdmField<float>            m_symbolSkipPixelDistance;


    caf::PdmField< caf::AppEnum< PointSymbolEnum > > m_pointSymbol;
    caf::PdmField< caf::AppEnum< LineStyleEnum > >   m_lineStyle;
    caf::PdmField< CurveInterpolation >              m_curveInterpolation;
};


