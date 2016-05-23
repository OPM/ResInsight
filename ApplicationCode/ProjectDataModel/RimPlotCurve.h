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
public:
    RimPlotCurve();
    virtual ~RimPlotCurve();

    void                            loadDataAndUpdate();

    void                            setParentQwtPlot(QwtPlot* plot);
    void                            detachQwtCurve();
    QwtPlotCurve*                   qwtPlotCurve() const;

    void                            setColor(const cvf::Color3f& color);
    bool                            isCurveVisible() const;
    QString                         curveName() const { return m_curveName; }

protected:

    virtual QString                 createCurveAutoName() = 0;
    virtual void                    zoomAllParentPlot() = 0;
    virtual void                    onLoadDataAndUpdate() = 0;

    void                            updateCurvePresentation();
    void                            updateCurveName();
    void                            updateCurveVisibility();
    void                            updateCurveAppearance();

    void                            updateOptionSensitivity();

protected:

    // Overridden PDM methods
    virtual void                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual caf::PdmFieldHandle*    objectToggleField();
    virtual caf::PdmFieldHandle*    userDescriptionField();
    virtual void                    initAfterRead();
    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly);

protected:
    QPointer<QwtPlot>               m_parentQwtPlot;
    RiuLineSegmentQwtPlotCurve*     m_qwtPlotCurve;

    caf::PdmField<bool>             m_showCurve;
    caf::PdmField<QString>          m_curveName;
    caf::PdmField<QString>          m_customCurveName;

    caf::PdmField<bool>             m_isUsingAutoName;
    caf::PdmField<cvf::Color3f>     m_curveColor;
    caf::PdmField<float>            m_curveThickness;

    caf::PdmField< caf::AppEnum< PointSymbolEnum > > m_pointSymbol;
    caf::PdmField< caf::AppEnum< LineStyleEnum > >   m_lineStyle;
};


