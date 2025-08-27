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
#include "cafPdmUiCommandSystemProxy.h"
#include "cafPdmUiDoubleSliderEditor.h"

CAF_PDM_XML_ABSTRACT_SOURCE_INIT( RimPlotCurveAppearance, "PlotCurveAppearance" );

namespace caf
{
template <>
void RimPlotCurveAppearance::PointSymbol::setUp()
{
    addItem( RiuPlotCurveSymbol::SYMBOL_NONE, "SYMBOL_NONE", "None" );
    addItem( RiuPlotCurveSymbol::SYMBOL_ELLIPSE, "SYMBOL_ELLIPSE", "Ellipse" );
    addItem( RiuPlotCurveSymbol::SYMBOL_RECT, "SYMBOL_RECT", "Rect" );
    addItem( RiuPlotCurveSymbol::SYMBOL_DIAMOND, "SYMBOL_DIAMOND", "Diamond" );
    addItem( RiuPlotCurveSymbol::SYMBOL_TRIANGLE, "SYMBOL_TRIANGLE", "Triangle" );
    addItem( RiuPlotCurveSymbol::SYMBOL_DOWN_TRIANGLE, "SYMBOL_DOWN_TRIANGLE", "Down Triangle" );
    addItem( RiuPlotCurveSymbol::SYMBOL_CROSS, "SYMBOL_CROSS", "Cross" );
    addItem( RiuPlotCurveSymbol::SYMBOL_XCROSS, "SYMBOL_XCROSS", "X Cross" );
    addItem( RiuPlotCurveSymbol::SYMBOL_STAR1, "SYMBOL_STAR1", "Star 1" );
    addItem( RiuPlotCurveSymbol::SYMBOL_STAR2, "SYMBOL_STAR2", "Star 2" );
    addItem( RiuPlotCurveSymbol::SYMBOL_HEXAGON, "SYMBOL_HEXAGON", "Hexagon" );
    addItem( RiuPlotCurveSymbol::SYMBOL_LEFT_TRIANGLE, "SYMBOL_LEFT_TRIANGLE", "Left Triangle" );
    addItem( RiuPlotCurveSymbol::SYMBOL_RIGHT_TRIANGLE, "SYMBOL_RIGHT_TRIANGLE", "Right Triangle" );
    setDefault( RiuPlotCurveSymbol::SYMBOL_NONE );
}

template <>
void RimPlotCurveAppearance::LabelPosition::setUp()
{
    addItem( RiuPlotCurveSymbol::LabelAboveSymbol, "LABEL_ABOVE_SYMBOL", "Label above Symbol" );
    addItem( RiuPlotCurveSymbol::LabelBelowSymbol, "LABEL_BELOW_SYMBOL", "Label below Symbol" );
    addItem( RiuPlotCurveSymbol::LabelLeftOfSymbol, "LABEL_LEFT_OF_SYMBOL", "Label left of Symbol" );
    addItem( RiuPlotCurveSymbol::LabelRightOfSymbol, "LABEL_RIGHT_OF_SYMBOL", "Label right of Symbol" );
    setDefault( RiuPlotCurveSymbol::LabelAboveSymbol );
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
    setDefault( Qt::NoBrush );
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
    , m_curveFittingToleranceVisible( true )
{
    CAF_PDM_InitObject( "Curve Apperance" );

    CAF_PDM_InitField( &m_curveColor, "Color", RiaColorTools::textColor3f(), "Color" );

    CAF_PDM_InitField( &m_curveColorOpacity, "CurveColorOpacity", 1.0f, "Opacity" );
    m_curveColorOpacity.registerKeywordAlias( "CurveColorTransparency" );
    m_curveColorOpacity.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_fillColor, "FillColor", cvf::Color3f( -1.0, -1.0, -1.0 ), "Fill Color" );

    CAF_PDM_InitField( &m_fillColorOpacity, "FillColorOpacity", 1.0f, "Fill Color Opacity" );
    m_fillColorOpacity.registerKeywordAlias( "FillColorTransparency" );
    m_fillColorOpacity.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_curveThickness, "Thickness", 1, "Line Thickness" );
    m_curveThickness.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_curveInterpolation, "CurveInterpolation", "Interpolation" );
    CAF_PDM_InitFieldNoDefault( &m_lineStyle, "LineStyle", "Line Style" );
    CAF_PDM_InitFieldNoDefault( &m_fillStyle, "FillStyle", "Area Fill Style" );
    CAF_PDM_InitFieldNoDefault( &m_pointSymbol, "PointSymbol", "Symbol" );
    CAF_PDM_InitField( &m_symbolEdgeColor, "SymbolEdgeColor", RiaColorTools::textColor3f(), "Symbol Edge Color" );

    CAF_PDM_InitField( &m_symbolSkipPixelDistance, "SymbolSkipPxDist", 0.0f, "Symbol Skip Distance", "", "Minimum pixel distance between symbols", "" );

    CAF_PDM_InitField( &m_curveFittingTolerance,
                       "CurveFittingTolerance",
                       1.0f,
                       "Curve Fitting Tolerance",
                       "",
                       "Value above 0 : Curve fitting tolerance (default 1.0), 0 : disable curve fitting" );

    CAF_PDM_InitFieldNoDefault( &m_symbolLabel, "SymbolLabel", "Symbol Label" );
    CAF_PDM_InitField( &m_symbolSize, "SymbolSize", 6, "Symbol Size" );

    CAF_PDM_InitFieldNoDefault( &m_symbolLabelPosition, "SymbolLabelPosition", "Symbol Label Position" );
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
void RimPlotCurveAppearance::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( &m_pointSymbol == changedField )
    {
        m_symbolSize.uiCapability()->setUiReadOnly( m_pointSymbol() == RiuPlotCurveSymbol::SYMBOL_NONE );
        m_symbolSkipPixelDistance.uiCapability()->setUiReadOnly( m_pointSymbol() == RiuPlotCurveSymbol::SYMBOL_NONE );
    }
    else if ( &m_lineStyle == changedField )
    {
        m_curveThickness.uiCapability()->setUiReadOnly( m_lineStyle() == RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_NONE );
        m_curveInterpolation.uiCapability()->setUiReadOnly( m_lineStyle() == RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_NONE );
    }
    else if ( &m_fillColor == changedField )
    {
        fillColorChanged.send();
    }

    appearanceChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurveAppearance ::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_curveColorOpacity || field == &m_fillColorOpacity )
    {
        if ( auto myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute ) )
        {
            myAttr->m_minimum  = 0.0;
            myAttr->m_maximum  = 1.0;
            myAttr->m_decimals = 2;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurveAppearance::initAfterRead()
{
    m_symbolSize.uiCapability()->setUiReadOnly( m_pointSymbol() == RiuPlotCurveSymbol::SYMBOL_NONE );
    m_symbolSkipPixelDistance.uiCapability()->setUiReadOnly( m_pointSymbol() == RiuPlotCurveSymbol::SYMBOL_NONE );
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
void RimPlotCurveAppearance::setColorWithFieldChanged( const QColor& color )
{
    caf::PdmUiCommandSystemProxy::instance()->setUiValueToField( m_curveColor.uiCapability(), color );
    caf::PdmUiCommandSystemProxy::instance()->setUiValueToField( m_fillColor.uiCapability(), color );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurveAppearance::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_curveColor );
    uiOrdering.add( &m_curveColorOpacity );
    m_curveColor.uiCapability()->setUiHidden( !m_colorVisible );
    m_curveColorOpacity.uiCapability()->setUiHidden( !m_colorVisible );

    uiOrdering.add( &m_pointSymbol );
    if ( RiuPlotCurveSymbol::isFilledSymbol( m_pointSymbol() ) )
    {
        uiOrdering.add( &m_symbolEdgeColor );
    }
    uiOrdering.add( &m_symbolSize );
    uiOrdering.add( &m_symbolSkipPixelDistance );
    uiOrdering.add( &m_lineStyle );
    uiOrdering.add( &m_curveThickness );

    if ( m_curveFittingToleranceVisible )
    {
        uiOrdering.add( &m_curveFittingTolerance );
        m_curveFittingTolerance.uiCapability()->setUiReadOnly( m_lineStyle() == RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_SOLID );
    }

    uiOrdering.add( &m_fillStyle );
    m_fillStyle.uiCapability()->setUiHidden( !m_fillOptionsVisible );

    if ( m_fillStyle != Qt::BrushStyle::NoBrush )
    {
        uiOrdering.add( &m_fillColor );
        uiOrdering.add( &m_fillColorOpacity );
    }
    m_fillColor.uiCapability()->setUiHidden( !m_fillOptionsVisible );
    m_fillColorOpacity.uiCapability()->setUiHidden( !m_fillOptionsVisible );

    uiOrdering.add( &m_curveInterpolation );
    m_curveInterpolation.uiCapability()->setUiHidden( !m_interpolationVisible );

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimPlotCurveAppearance::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
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
void RimPlotCurveAppearance::setSymbol( RiuPlotCurveSymbol::PointSymbolEnum symbolStyle )
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
RiuPlotCurveSymbol::PointSymbolEnum RimPlotCurveAppearance::symbol() const
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
void RimPlotCurveAppearance::setSymbolLabelPosition( RiuPlotCurveSymbol::LabelPosition labelPosition )
{
    m_symbolLabelPosition = labelPosition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotCurveSymbol::LabelPosition RimPlotCurveAppearance::symbolLabelPosition() const
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
    setSymbol( RiuPlotCurveSymbol::SYMBOL_NONE );
    setSymbolSkipDistance( 10 );
    setFillStyle( Qt::NoBrush );
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
void RimPlotCurveAppearance::setFillColorOpacity( float opacity )
{
    m_fillColorOpacity = opacity;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
float RimPlotCurveAppearance::fillColorOpacity() const
{
    return m_fillColorOpacity;
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
float RimPlotCurveAppearance::curveFittingTolerance() const
{
    return m_curveFittingTolerance();
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurveAppearance::setCurveFittingToleranceVisible( bool isVisible )
{
    m_curveFittingToleranceVisible = isVisible;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurveAppearance::setCurveColorOpacity( float opacity )
{
    m_curveColorOpacity = opacity;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
float RimPlotCurveAppearance::curveColorOpacity() const
{
    return m_curveColorOpacity;
}
