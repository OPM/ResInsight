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

#include "RimPlotCurveAppearance.h"

#include "RiaColorTools.h"

#include "cafPdmUiComboBoxEditor.h"

#include "cvfAssert.h"

CAF_PDM_XML_ABSTRACT_SOURCE_INIT( RimPlotCurveAppearance, "PlotCurveAppearance" );

namespace caf
{
template <>
void RimPlotCurveAppearance::PointSymbol::setUp()
{
    addItem( RiuQwtSymbol::SYMBOL_NONE, "SYMBOL_NONE", "None" );
    addItem( RiuQwtSymbol::SYMBOL_ELLIPSE, "SYMBOL_ELLIPSE", "Ellipse" );
    addItem( RiuQwtSymbol::SYMBOL_RECT, "SYMBOL_RECT", "Rect" );
    addItem( RiuQwtSymbol::SYMBOL_DIAMOND, "SYMBOL_DIAMOND", "Diamond" );
    addItem( RiuQwtSymbol::SYMBOL_TRIANGLE, "SYMBOL_TRIANGLE", "Triangle" );
    addItem( RiuQwtSymbol::SYMBOL_DOWN_TRIANGLE, "SYMBOL_DOWN_TRIANGLE", "Down Triangle" );
    addItem( RiuQwtSymbol::SYMBOL_CROSS, "SYMBOL_CROSS", "Cross" );
    addItem( RiuQwtSymbol::SYMBOL_XCROSS, "SYMBOL_XCROSS", "X Cross" );
    addItem( RiuQwtSymbol::SYMBOL_STAR1, "SYMBOL_STAR1", "Star 1" );
    addItem( RiuQwtSymbol::SYMBOL_STAR2, "SYMBOL_STAR2", "Star 2" );
    addItem( RiuQwtSymbol::SYMBOL_HEXAGON, "SYMBOL_HEXAGON", "Hexagon" );
    addItem( RiuQwtSymbol::SYMBOL_LEFT_TRIANGLE, "SYMBOL_LEFT_TRIANGLE", "Left Triangle" );
    addItem( RiuQwtSymbol::SYMBOL_RIGHT_TRIANGLE, "SYMBOL_RIGHT_TRIANGLE", "Right Triangle" );
    setDefault( RiuQwtSymbol::SYMBOL_NONE );
}

template <>
void RimPlotCurveAppearance::LabelPosition::setUp()
{
    addItem( RiuQwtSymbol::LabelAboveSymbol, "LABEL_ABOVE_SYMBOL", "Label above Symbol" );
    addItem( RiuQwtSymbol::LabelBelowSymbol, "LABEL_BELOW_SYMBOL", "Label below Symbol" );
    addItem( RiuQwtSymbol::LabelLeftOfSymbol, "LABEL_LEFT_OF_SYMBOL", "Label left of Symbol" );
    addItem( RiuQwtSymbol::LabelRightOfSymbol, "LABEL_RIGHT_OF_SYMBOL", "Label right of Symbol" );
    setDefault( RiuQwtSymbol::LabelAboveSymbol );
}

template <>
void RimPlotCurveAppearance::FillStyle::setUp()
{
    addItem( Qt::NoBrush, "NO_FILL", "No Fill" );
    addItem( Qt::SolidPattern, "SOLID_FILL", "Solid Fill" );
    addItem( Qt::Dense1Pattern, "DENSE_FILL", "Dense Pattern" );
    addItem( Qt::Dense7Pattern, "SPARSE_FILL", "Sparse Pattern" );
    addItem( Qt::HorPattern, "HOR_FILL", "Horizontal Lines" );
    addItem( Qt::VerPattern, "VER_FILL", "Vertical Lines" );
    addItem( Qt::BDiagPattern, "DIAG_FILL", "Diagonal Lines" );
    addItem( Qt::CrossPattern, "CROSS_FILL", "Mesh" );
    addItem( Qt::DiagCrossPattern, "DIAG_CROSS_FILL", "Diagonal Mesh" );
}

} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotCurveAppearance::RimPlotCurveAppearance()
    : appearanceChanged( this )
    , fillColorChanged( this )
    , m_colorVisible( true )
    , m_interpolationVisible( true )
    , m_fillOptionsVisible( true )
{
    CAF_PDM_InitObject( "Curve Apperance", "", "", "" );

    CAF_PDM_InitField( &m_curveColor, "Color", RiaColorTools::textColor3f(), "Color", "", "", "" );
    CAF_PDM_InitField( &m_fillColor, "FillColor", cvf::Color3f( -1.0, -1.0, -1.0 ), "Fill Color", "", "", "" );

    CAF_PDM_InitField( &m_curveThickness, "Thickness", 1, "Line Thickness", "", "", "" );
    m_curveThickness.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_curveInterpolation, "CurveInterpolation", "Interpolation", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_lineStyle, "LineStyle", "Line Style", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_fillStyle, "FillStyle", "Area Fill Style", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_pointSymbol, "PointSymbol", "Symbol", "", "", "" );
    CAF_PDM_InitField( &m_symbolEdgeColor, "SymbolEdgeColor", RiaColorTools::textColor3f(), "Symbol Edge Color", "", "", "" );

    CAF_PDM_InitField( &m_symbolSkipPixelDistance,
                       "SymbolSkipPxDist",
                       0.0f,
                       "Symbol Skip Distance",
                       "",
                       "Minimum pixel distance between symbols",
                       "" );

    CAF_PDM_InitFieldNoDefault( &m_symbolLabel, "SymbolLabel", "Symbol Label", "", "", "" );
    CAF_PDM_InitField( &m_symbolSize, "SymbolSize", 6, "Symbol Size", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_symbolLabelPosition, "SymbolLabelPosition", "Symbol Label Position", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotCurveAppearance::~RimPlotCurveAppearance()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurveAppearance::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                               const QVariant&            oldValue,
                                               const QVariant&            newValue )
{
    if ( &m_curveColor == changedField || &m_curveThickness == changedField || &m_pointSymbol == changedField ||
         &m_lineStyle == changedField || &m_symbolSkipPixelDistance == changedField ||
         &m_curveInterpolation == changedField || &m_symbolSize == changedField || &m_symbolEdgeColor == changedField ||
         &m_fillStyle == changedField || &m_fillColor == changedField )
    {
        if ( &m_pointSymbol == changedField )
        {
            m_symbolSize.uiCapability()->setUiReadOnly( m_pointSymbol() == RiuQwtSymbol::SYMBOL_NONE );
            m_symbolSkipPixelDistance.uiCapability()->setUiReadOnly( m_pointSymbol() == RiuQwtSymbol::SYMBOL_NONE );
        }
        else if ( &m_lineStyle == changedField )
        {
            m_curveThickness.uiCapability()->setUiReadOnly( m_lineStyle() ==
                                                            RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_NONE );
            m_curveInterpolation.uiCapability()->setUiReadOnly( m_lineStyle() ==
                                                                RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_NONE );
        }
        else if ( &m_fillColor == changedField )
        {
            fillColorChanged.send();
        }

        appearanceChanged.send();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurveAppearance::initAfterRead()
{
    m_symbolSize.uiCapability()->setUiReadOnly( m_pointSymbol() == RiuQwtSymbol::SYMBOL_NONE );
    m_symbolSkipPixelDistance.uiCapability()->setUiReadOnly( m_pointSymbol() == RiuQwtSymbol::SYMBOL_NONE );
    m_curveThickness.uiCapability()->setUiReadOnly( m_lineStyle() == RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_NONE );
    m_curveInterpolation.uiCapability()->setUiReadOnly( m_lineStyle() == RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_NONE );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurveAppearance::setColor( const cvf::Color3f& color )
{
    m_curveColor = color;
    m_fillColor  = color;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimPlotCurveAppearance::color() const
{
    return m_curveColor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurveAppearance::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_curveColor );
    m_curveColor.uiCapability()->setUiHidden( !m_colorVisible );

    uiOrdering.add( &m_pointSymbol );
    if ( RiuQwtSymbol::isFilledSymbol( m_pointSymbol() ) )
    {
        uiOrdering.add( &m_symbolEdgeColor );
    }
    uiOrdering.add( &m_symbolSize );
    uiOrdering.add( &m_symbolSkipPixelDistance );
    uiOrdering.add( &m_lineStyle );
    uiOrdering.add( &m_curveThickness );

    uiOrdering.add( &m_fillStyle );
    m_fillStyle.uiCapability()->setUiHidden( !m_fillOptionsVisible );

    if ( m_fillStyle != Qt::BrushStyle::NoBrush )
    {
        uiOrdering.add( &m_fillColor );
    }
    m_fillColor.uiCapability()->setUiHidden( !m_fillOptionsVisible );

    uiOrdering.add( &m_curveInterpolation );
    m_curveInterpolation.uiCapability()->setUiHidden( !m_interpolationVisible );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimPlotCurveAppearance::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_curveThickness )
    {
        for ( size_t i = 0; i < 10; i++ )
        {
            options.push_back( caf::PdmOptionItemInfo( QString::number( i + 1 ), QVariant::fromValue( i + 1 ) ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurveAppearance::setLineStyle( RiuQwtPlotCurveDefines::LineStyleEnum lineStyle )
{
    m_lineStyle = lineStyle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurveAppearance::setSymbol( RiuQwtSymbol::PointSymbolEnum symbolStyle )
{
    m_pointSymbol = symbolStyle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurveAppearance::setInterpolation( RiuQwtPlotCurveDefines::CurveInterpolationEnum curveInterpolation )
{
    m_curveInterpolation = curveInterpolation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtPlotCurveDefines::CurveInterpolationEnum RimPlotCurveAppearance::interpolation() const
{
    return m_curveInterpolation();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtPlotCurveDefines::LineStyleEnum RimPlotCurveAppearance::lineStyle() const
{
    return m_lineStyle();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtSymbol::PointSymbolEnum RimPlotCurveAppearance::symbol() const
{
    return m_pointSymbol();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimPlotCurveAppearance::symbolSize() const
{
    return m_symbolSize();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimPlotCurveAppearance::symbolEdgeColor() const
{
    return m_symbolEdgeColor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
float RimPlotCurveAppearance::symbolSkipDistance() const
{
    return m_symbolSkipPixelDistance;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurveAppearance::setSymbolEdgeColor( const cvf::Color3f& edgeColor )
{
    m_symbolEdgeColor = edgeColor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurveAppearance::setSymbolSkipDistance( float distance )
{
    m_symbolSkipPixelDistance = distance;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurveAppearance::setSymbolLabel( const QString& label )
{
    m_symbolLabel = label;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimPlotCurveAppearance::symbolLabel() const
{
    return m_symbolLabel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurveAppearance::setSymbolLabelPosition( RiuQwtSymbol::LabelPosition labelPosition )
{
    m_symbolLabelPosition = labelPosition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtSymbol::LabelPosition RimPlotCurveAppearance::symbolLabelPosition() const
{
    return m_symbolLabelPosition.value();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurveAppearance::setSymbolSize( int sizeInPixels )
{
    m_symbolSize = sizeInPixels;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurveAppearance::setLineThickness( int thickness )
{
    m_curveThickness = thickness;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimPlotCurveAppearance::lineThickness() const
{
    return m_curveThickness;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurveAppearance::resetAppearance()
{
    setColor( RiaColorTools::textColor3f() );
    setSymbolEdgeColor( RiaColorTools::textColor3f() );
    setLineThickness( 2 );
    setLineStyle( RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_SOLID );
    setSymbol( RiuQwtSymbol::SYMBOL_NONE );
    setSymbolSkipDistance( 10 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Qt::BrushStyle RimPlotCurveAppearance::fillStyle() const
{
    return m_fillStyle();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurveAppearance::setFillStyle( Qt::BrushStyle brushStyle )
{
    m_fillStyle = brushStyle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurveAppearance::setFillColor( const cvf::Color3f& fillColor )
{
    m_fillColor = fillColor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimPlotCurveAppearance::fillColor() const
{
    return m_fillColor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurveAppearance::setColorVisible( bool isVisible )
{
    m_colorVisible = isVisible;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurveAppearance::setInterpolationVisible( bool isVisible )
{
    m_interpolationVisible = isVisible;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurveAppearance::setFillOptionsVisible( bool isVisible )
{
    m_fillOptionsVisible = isVisible;
}
