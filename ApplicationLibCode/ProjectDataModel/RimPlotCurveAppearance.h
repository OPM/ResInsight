/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RiaCurveDataTools.h"
#include "RiaDefines.h"

#include "RiuPlotCurveSymbol.h"
#include "RiuQwtPlotCurveDefines.h"

#include "cafPdmField.h"
#include "cafPdmFieldCvfColor.h"
#include "cafPdmObject.h"

//==================================================================================================
///
///
//==================================================================================================
class RimPlotCurveAppearance : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    caf::Signal<> appearanceChanged;
    caf::Signal<> fillColorChanged;

public:
    using CurveInterpolation = caf::AppEnum<RiuQwtPlotCurveDefines::CurveInterpolationEnum>;
    using LineStyle          = caf::AppEnum<RiuQwtPlotCurveDefines::LineStyleEnum>;
    using PointSymbol        = caf::AppEnum<RiuPlotCurveSymbol::PointSymbolEnum>;
    using LabelPosition      = caf::AppEnum<RiuPlotCurveSymbol::LabelPosition>;
    using FillStyle          = caf::AppEnum<Qt::BrushStyle>;

public:
    RimPlotCurveAppearance();
    ~RimPlotCurveAppearance() override;

    void         setColor( const cvf::Color3f& color );
    cvf::Color3f color() const;

    void setColorWithFieldChanged( const QColor& color );

    void                                  setLineStyle( RiuQwtPlotCurveDefines::LineStyleEnum lineStyle );
    RiuQwtPlotCurveDefines::LineStyleEnum lineStyle() const;

    void setLineThickness( int thickness );
    int  lineThickness() const;

    void                                setSymbol( RiuPlotCurveSymbol::PointSymbolEnum symbolStyle );
    RiuPlotCurveSymbol::PointSymbolEnum symbol() const;

    void setSymbolSize( int sizeInPixels );
    int  symbolSize() const;

    cvf::Color3f symbolEdgeColor() const;
    void         setSymbolEdgeColor( const cvf::Color3f& edgeColor );

    void  setSymbolSkipDistance( float distance );
    float symbolSkipDistance() const;

    void    setSymbolLabel( const QString& label );
    QString symbolLabel() const;

    void                              setSymbolLabelPosition( RiuPlotCurveSymbol::LabelPosition labelPosition );
    RiuPlotCurveSymbol::LabelPosition symbolLabelPosition() const;

    void           resetAppearance();
    Qt::BrushStyle fillStyle() const;
    void           setFillStyle( Qt::BrushStyle brushStyle );

    void         setFillColor( const cvf::Color3f& fillColor );
    cvf::Color3f fillColor() const;

    void  setCurveColorOpacity( float opacity );
    float curveColorOpacity() const;

    void  setFillColorOpacity( float opacity );
    float fillColorOpacity() const;

    float curveFittingTolerance() const;

    void                                           setInterpolation( RiuQwtPlotCurveDefines::CurveInterpolationEnum );
    RiuQwtPlotCurveDefines::CurveInterpolationEnum interpolation() const;

    void setInterpolationVisible( bool isVisible );
    void setColorVisible( bool isVisible );
    void setFillOptionsVisible( bool isVisible );
    void setCurveFittingToleranceVisible( bool isVisible );

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

protected:
    // Overridden PDM methods
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

    void initAfterRead() override;

protected:
    caf::PdmField<QString> m_symbolLabel;
    caf::PdmField<int>     m_symbolSize;

    caf::PdmField<cvf::Color3f> m_curveColor;
    caf::PdmField<float>        m_curveColorOpacity;
    caf::PdmField<int>          m_curveThickness;
    caf::PdmField<float>        m_symbolSkipPixelDistance;

    caf::PdmField<float> m_curveFittingTolerance;

    caf::PdmField<PointSymbol>        m_pointSymbol;
    caf::PdmField<LineStyle>          m_lineStyle;
    caf::PdmField<FillStyle>          m_fillStyle;
    caf::PdmField<cvf::Color3f>       m_fillColor;
    caf::PdmField<float>              m_fillColorOpacity;
    caf::PdmField<CurveInterpolation> m_curveInterpolation;
    caf::PdmField<LabelPosition>      m_symbolLabelPosition;
    caf::PdmField<cvf::Color3f>       m_symbolEdgeColor;

    bool m_colorVisible;
    bool m_interpolationVisible;
    bool m_fillOptionsVisible;
    bool m_curveFittingToleranceVisible;
};
