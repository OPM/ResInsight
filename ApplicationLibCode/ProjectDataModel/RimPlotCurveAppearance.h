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

#include "RiuQwtPlotCurveDefines.h"
#include "RiuQwtSymbol.h"

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

public:
    typedef caf::AppEnum<RiuQwtPlotCurveDefines::CurveInterpolationEnum> CurveInterpolation;
    typedef caf::AppEnum<RiuQwtPlotCurveDefines::LineStyleEnum>          LineStyle;
    typedef caf::AppEnum<RiuQwtSymbol::PointSymbolEnum>                  PointSymbol;
    typedef caf::AppEnum<RiuQwtSymbol::LabelPosition>                    LabelPosition;
    typedef caf::AppEnum<Qt::BrushStyle>                                 FillStyle;

public:
    RimPlotCurveAppearance();
    ~RimPlotCurveAppearance() override;

    void         setColor( const cvf::Color3f& color );
    cvf::Color3f color() const { return m_curveColor; }

    void                                  setLineStyle( RiuQwtPlotCurveDefines::LineStyleEnum lineStyle );
    RiuQwtPlotCurveDefines::LineStyleEnum lineStyle() const;

    void setLineThickness( int thickness );
    int  lineThickness() const;

    void                          setSymbol( RiuQwtSymbol::PointSymbolEnum symbolStyle );
    RiuQwtSymbol::PointSymbolEnum symbol() const;

    void setSymbolSize( int sizeInPixels );
    int  symbolSize() const;

    cvf::Color3f symbolEdgeColor() const;
    void         setSymbolEdgeColor( const cvf::Color3f& edgeColor );

    void  setSymbolSkipDistance( float distance );
    float symbolSkipDistance() const;

    void setSymbolLabel( const QString& label );

    void                        setSymbolLabelPosition( RiuQwtSymbol::LabelPosition labelPosition );
    RiuQwtSymbol::LabelPosition systemLabelPosition();

    void           resetAppearance();
    Qt::BrushStyle fillStyle() const;
    void           setFillStyle( Qt::BrushStyle brushStyle );
    void           setFillColor( const cvf::Color3f& fillColor );

    void setInterpolation( RiuQwtPlotCurveDefines::CurveInterpolationEnum );

    void setInterpolationVisible( bool isVisible );
    void setColorVisible( bool isVisible );
    void setFillOptionsVisible( bool isVisible );

    void appearanceUiOrdering( caf::PdmUiOrdering& uiOrdering );

protected:
    // Overridden PDM methods
    void                          fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;
    void                          initAfterRead() override;

protected:
    caf::PdmField<QString> m_symbolLabel;
    caf::PdmField<int>     m_symbolSize;
    caf::PdmField<QString> m_legendEntryText;

    caf::PdmField<cvf::Color3f> m_curveColor;
    caf::PdmField<int>          m_curveThickness;
    caf::PdmField<float>        m_symbolSkipPixelDistance;

    caf::PdmField<PointSymbol>        m_pointSymbol;
    caf::PdmField<LineStyle>          m_lineStyle;
    caf::PdmField<FillStyle>          m_fillStyle;
    caf::PdmField<cvf::Color3f>       m_fillColor;
    caf::PdmField<CurveInterpolation> m_curveInterpolation;
    caf::PdmField<LabelPosition>      m_symbolLabelPosition;
    caf::PdmField<cvf::Color3f>       m_symbolEdgeColor;

    bool m_colorVisible;
    bool m_interpolationVisible;
    bool m_fillOptionsVisible;
};
