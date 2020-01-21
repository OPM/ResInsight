/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RiuWellPathComponentPlotItem.h"

#include "RiaColorTables.h"
#include "RiaColorTools.h"

#include "RimFishbonesMultipleSubs.h"
#include "RimFracture.h"
#include "RimFractureTemplate.h"
#include "RimPerforationInterval.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"
#include "RimWellPathAttribute.h"
#include "RimWellPathAttributeCollection.h"
#include "RimWellPathValve.h"

#include "RigWellPath.h"

#include "qwt_plot.h"
#include "qwt_plot_marker.h"
#include "qwt_plot_shapeitem.h"

#include <QBrush>
#include <Qt>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWellPathComponentPlotItem::RiuWellPathComponentPlotItem( const RimWellPath* wellPath )
    : m_wellPath( wellPath )
    , m_componentType( RiaDefines::WELL_PATH )
    , m_columnOffset( 0.0 )
    , m_depthType( RiaDefines::MEASURED_DEPTH )
    , m_maxColumnOffset( 0.0 )
    , m_showLabel( false )
{
    CVF_ASSERT( wellPath && wellPath->wellPathGeometry() );
    double wellStart = wellPath->wellPathGeometry()->measureDepths().front();
    double wellEnd   = wellPath->wellPathGeometry()->measureDepths().back();
    m_startMD        = wellStart;
    m_endMD          = wellEnd;
    m_label          = wellPath->name();
    m_legendTitle    = "Well Tube";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWellPathComponentPlotItem::RiuWellPathComponentPlotItem( const RimWellPath*                   wellPath,
                                                            const RimWellPathComponentInterface* component )
    : m_wellPath( wellPath )
    , m_columnOffset( 0.0 )
    , m_depthType( RiaDefines::MEASURED_DEPTH )
    , m_maxColumnOffset( 0.0 )
    , m_showLabel( false )
{
    CVF_ASSERT( wellPath && component );

    m_componentType = component->componentType();
    m_label         = component->componentLabel();
    m_legendTitle   = component->componentTypeLabel();
    m_startMD       = component->startMD();
    m_endMD         = component->endMD();

    calculateColumnOffsets( component );
    const RimWellPathValve* valve = dynamic_cast<const RimWellPathValve*>( component );
    if ( valve )
    {
        m_subMDs = valve->valveLocations();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWellPathComponentPlotItem::~RiuWellPathComponentPlotItem()
{
    detachFromQwt();

    if ( m_parentQwtPlot )
    {
        m_parentQwtPlot->replot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuWellPathComponentPlotItem::label() const
{
    return m_label;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellPathComponentPlotItem::loadDataAndUpdate( bool updateParentPlot )
{
    onLoadDataAndUpdate( updateParentPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::WellPathComponentType RiuWellPathComponentPlotItem::componentType() const
{
    return m_componentType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellPathComponentPlotItem::calculateColumnOffsets( const RimWellPathComponentInterface* component )
{
    std::set<double> uniqueCasingDiameters;

    std::vector<RimWellPathAttributeCollection*> attributeCollection;
    m_wellPath->descendantsIncludingThisOfType( attributeCollection );
    for ( const RimWellPathAttribute* otherAttribute : attributeCollection.front()->attributes() )
    {
        if ( otherAttribute->componentType() == RiaDefines::CASING )
        {
            uniqueCasingDiameters.insert( otherAttribute->diameterInInches() );
        }
    }

    if ( !uniqueCasingDiameters.empty() )
    {
        m_maxColumnOffset = ( uniqueCasingDiameters.size() - 1 ) * 0.25;

        const RimWellPathAttribute* myAttribute = dynamic_cast<const RimWellPathAttribute*>( component );
        if ( myAttribute && myAttribute->componentType() == RiaDefines::CASING )
        {
            int nNarrowerCasings = std::count_if( uniqueCasingDiameters.begin(),
                                                  uniqueCasingDiameters.end(),
                                                  [myAttribute]( double otherCasingDiameter ) {
                                                      return otherCasingDiameter < myAttribute->diameterInInches();
                                                  } );

            m_columnOffset = nNarrowerCasings * 0.25;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellPathComponentPlotItem::onLoadDataAndUpdate( bool updateParentPlot )
{
    double startDepth, endDepth;
    std::tie( startDepth, endDepth ) = depthsOfDepthType();
    double midDepth                  = 0.5 * ( startDepth + endDepth );

    double casingTrackEnd = 0.75 + m_maxColumnOffset;

    if ( m_componentType == RiaDefines::WELL_PATH )
    {
        addColumnFeature( -0.25, 0.25, startDepth, endDepth, componentColor() );
    }
    else if ( m_componentType == RiaDefines::CASING )
    {
        double posMin = 0.5 + m_columnOffset;
        double posMax = 0.75 + m_columnOffset;
        addColumnFeature( -posMax, -posMin, startDepth, endDepth, componentColor() );
        addColumnFeature( posMin, posMax, startDepth, endDepth, componentColor() );
        addMarker( -posMax, endDepth, 12, RiuQwtSymbol::SYMBOL_LEFT_ANGLED_TRIANGLE, componentColor() );
        addMarker( posMax, endDepth, 12, RiuQwtSymbol::SYMBOL_RIGHT_ANGLED_TRIANGLE, componentColor() );
        addMarker( casingTrackEnd, endDepth, 12, RiuQwtSymbol::SYMBOL_RIGHT_ANGLED_TRIANGLE, componentColor( 0.0 ), label() );
    }
    else if ( m_componentType == RiaDefines::LINER )
    {
        addColumnFeature( -0.5, -0.25, startDepth, endDepth, componentColor() );
        addColumnFeature( 0.25, 0.5, startDepth, endDepth, componentColor() );
        addMarker( casingTrackEnd, endDepth, 10, RiuQwtSymbol::SYMBOL_RIGHT_ANGLED_TRIANGLE, componentColor( 0.0 ), label() );
    }
    else if ( m_componentType == RiaDefines::PERFORATION_INTERVAL )
    {
        addColumnFeature( -casingTrackEnd, -0.25, startDepth, endDepth, componentColor(), Qt::Dense6Pattern );
        addColumnFeature( 0.25, casingTrackEnd, startDepth, endDepth, componentColor(), Qt::Dense6Pattern );
        // Empirically a spacing of around 30 in depth between symbols looks good in the most relevant zoom levels.
        const double markerSpacing = 30;
        const int    markerSize    = 6;
        double       markerDepth   = startDepth;
        while ( markerDepth < endDepth - 5 )
        {
            addMarker( -casingTrackEnd,
                       markerDepth,
                       markerSize,
                       RiuQwtSymbol::SYMBOL_LEFT_ALIGNED_TRIANGLE,
                       componentColor() );
            addMarker( casingTrackEnd,
                       markerDepth,
                       markerSize,
                       RiuQwtSymbol::SYMBOL_RIGHT_ALIGNED_TRIANGLE,
                       componentColor() );

            markerDepth += markerSpacing;
        }
        addMarker( casingTrackEnd, midDepth, 10, RiuQwtSymbol::SYMBOL_RIGHT_ALIGNED_TRIANGLE, componentColor( 0.0 ), label() );

        QwtPlotItem* legendItem1 =
            createMarker( 16.0, 0.0, 6, RiuQwtSymbol::SYMBOL_RIGHT_ALIGNED_TRIANGLE, componentColor() );
        legendItem1->setLegendIconSize( QSize( 4, 8 ) );
        QwtPlotItem* legendItem2 =
            createMarker( 16.0, 8.0, 6, RiuQwtSymbol::SYMBOL_RIGHT_ALIGNED_TRIANGLE, componentColor() );
        legendItem2->setLegendIconSize( QSize( 4, 8 ) );
        m_combinedComponentGroup.addLegendItem( legendItem1 );
        m_combinedComponentGroup.addLegendItem( legendItem2 );
    }
    else if ( m_componentType == RiaDefines::FISHBONES )
    {
        addColumnFeature( -casingTrackEnd, -0.25, startDepth, endDepth, componentColor(), Qt::BDiagPattern );
        addColumnFeature( 0.25, casingTrackEnd, startDepth, endDepth, componentColor(), Qt::FDiagPattern );
        addMarker( casingTrackEnd, midDepth, 10, RiuQwtSymbol::SYMBOL_RIGHT_ANGLED_TRIANGLE, componentColor( 0.0 ), label() );
    }
    else if ( m_componentType == RiaDefines::FRACTURE )
    {
        addColumnFeature( -casingTrackEnd, -0.25, startDepth, endDepth, componentColor(), Qt::SolidPattern );
        addColumnFeature( 0.25, casingTrackEnd, startDepth, endDepth, componentColor(), Qt::SolidPattern );
        addMarker( casingTrackEnd,
                   startDepth,
                   10,
                   RiuQwtSymbol::SYMBOL_NONE,
                   componentColor(),
                   "",
                   Qt::AlignTop | Qt::AlignRight,
                   Qt::Horizontal,
                   true );
        addMarker( casingTrackEnd,
                   endDepth,
                   10,
                   RiuQwtSymbol::SYMBOL_NONE,
                   componentColor(),
                   "",
                   Qt::AlignTop | Qt::AlignRight,
                   Qt::Horizontal,
                   true );
        addMarker( casingTrackEnd,
                   startDepth,
                   1,
                   RiuQwtSymbol::SYMBOL_RIGHT_ANGLED_TRIANGLE,
                   componentColor( 0.0f ),
                   label(),
                   Qt::AlignTop | Qt::AlignRight );
    }
    else if ( m_componentType == RiaDefines::ICD )
    {
        for ( double md : m_subMDs )
        {
            addMarker( 0.0,
                       md,
                       16,
                       RiuQwtSymbol::SYMBOL_ELLIPSE,
                       componentColor(),
                       "",
                       Qt::AlignCenter,
                       Qt::Horizontal,
                       false,
                       true );
        }
        m_combinedComponentGroup.addLegendItem(
            createMarker( 0.0, 0.0, 12.0, RiuQwtSymbol::SYMBOL_ELLIPSE, componentColor() ) );
    }
    else if ( m_componentType == RiaDefines::ICV )
    {
        for ( double md : m_subMDs )
        {
            addMarker( 0.0,
                       md,
                       16,
                       RiuQwtSymbol::SYMBOL_ELLIPSE,
                       componentColor(),
                       "",
                       Qt::AlignCenter,
                       Qt::Horizontal,
                       false,
                       true );
        }
        m_combinedComponentGroup.addLegendItem(
            createMarker( 0.0, 0.0, 12.0, RiuQwtSymbol::SYMBOL_ELLIPSE, componentColor() ) );
    }
    else if ( m_componentType == RiaDefines::AICD )
    {
        for ( double md : m_subMDs )
        {
            addMarker( 0.0,
                       md,
                       16,
                       RiuQwtSymbol::SYMBOL_ELLIPSE,
                       componentColor(),
                       "",
                       Qt::AlignCenter,
                       Qt::Horizontal,
                       false,
                       true );
        }
        m_combinedComponentGroup.addLegendItem(
            createMarker( 0.0, 0.0, 12.0, RiuQwtSymbol::SYMBOL_ELLIPSE, componentColor() ) );
    }
    else if ( m_componentType == RiaDefines::PACKER )
    {
        addColumnFeature( -casingTrackEnd, -0.25, startDepth, endDepth, componentColor(), Qt::DiagCrossPattern );
        addColumnFeature( 0.25, casingTrackEnd, startDepth, endDepth, componentColor(), Qt::DiagCrossPattern );
        addMarker( casingTrackEnd, midDepth, 10, RiuQwtSymbol::SYMBOL_RIGHT_ANGLED_TRIANGLE, componentColor( 0.0 ), label() );
    }
    m_combinedComponentGroup.setTitle( legendTitle() );
    m_combinedComponentGroup.setLegendIconSize( QSize( 20, 16 ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RiuWellPathComponentPlotItem::depthsOfDepthType() const
{
    double startDepth = m_startMD;
    double endDepth   = m_endMD;

    if ( m_depthType == RiaDefines::TRUE_VERTICAL_DEPTH || m_depthType == RiaDefines::TRUE_VERTICAL_DEPTH_RKB )
    {
        cvf::Vec3d startPoint = m_wellPath->wellPathGeometry()->interpolatedPointAlongWellPath( m_startMD );
        cvf::Vec3d endPoint   = m_wellPath->wellPathGeometry()->interpolatedPointAlongWellPath( m_endMD );
        startDepth            = -startPoint.z();
        endDepth              = -endPoint.z();
        if ( m_depthType == RiaDefines::TRUE_VERTICAL_DEPTH_RKB )
        {
            startDepth += m_wellPath->wellPathGeometry()->rkbDiff();
            endDepth += m_wellPath->wellPathGeometry()->rkbDiff();
        }
    }
    return std::make_pair( startDepth, endDepth );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellPathComponentPlotItem::addMarker( double                        posX,
                                              double                        depth,
                                              int                           size,
                                              RiuQwtSymbol::PointSymbolEnum symbolType,
                                              cvf::Color4f                  baseColor,
                                              const QString&                label /*= QString("")*/,
                                              Qt::Alignment                 labelAlignment /*= Qt::AlignTop*/,
                                              Qt::Orientation               labelOrientation /*= Qt::Vertical*/,
                                              bool                          drawLine /*= false*/,
                                              bool                          contrastTextColor /*= true*/ )
{
    QwtPlotItem* marker = createMarker( posX,
                                        depth,
                                        size,
                                        symbolType,
                                        baseColor,
                                        label,
                                        labelAlignment,
                                        labelOrientation,
                                        drawLine,
                                        contrastTextColor );
    m_combinedComponentGroup.addPlotItem( marker );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QwtPlotItem* RiuWellPathComponentPlotItem::createMarker( double                        posX,
                                                         double                        depth,
                                                         int                           size,
                                                         RiuQwtSymbol::PointSymbolEnum symbolType,
                                                         cvf::Color4f                  baseColor,
                                                         const QString&                label /*= QString("")*/,
                                                         Qt::Alignment   labelAlignment /*= Qt::AlignTop*/,
                                                         Qt::Orientation labelOrientation /*= Qt::Vertical*/,
                                                         bool            drawLine /*= false*/,
                                                         bool            contrastTextColor /*= true*/ )
{
    QColor bgColor   = RiaColorTools::toQColor( baseColor );
    QColor textColor = RiaColorTools::toQColor( baseColor.toColor3f(), 1.0 );
    if ( contrastTextColor )
    {
        textColor = RiaColorTools::toQColor( RiaColorTools::contrastColor( baseColor.toColor3f() ) );
    }
    QwtPlotMarker* marker = new QwtPlotMarker( label );
    RiuQwtSymbol*  symbol = new RiuQwtSymbol( symbolType, "", RiuQwtSymbol::LabelRightOfSymbol );
    symbol->setSize( size );
    symbol->setColor( bgColor );
    marker->setSymbol( symbol );
    marker->setSpacing( 6 );
    marker->setXValue( posX );
    marker->setYValue( depth );

    if ( m_showLabel )
    {
        QwtText labelText( label );
        labelText.setColor( textColor );
        QFont font;
        font.setPointSize( 8 );
        labelText.setFont( font );
        marker->setLabel( labelText );
        marker->setLabelAlignment( labelAlignment );
        marker->setLabelOrientation( labelOrientation );
    }

    if ( drawLine )
    {
        marker->setLineStyle( QwtPlotMarker::HLine );
        marker->setLinePen( bgColor, 2.0, Qt::SolidLine );
    }
    return marker;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellPathComponentPlotItem::addColumnFeature( double         startX,
                                                     double         endX,
                                                     double         startDepth,
                                                     double         endDepth,
                                                     cvf::Color4f   baseColor,
                                                     Qt::BrushStyle brushStyle /*= Qt::SolidPattern*/ )
{
    if ( brushStyle != Qt::SolidPattern )
    {
        // If we're doing a special pattern, draw the background in white first over the existing pattern
        cvf::Color4f semiTransparentWhite( cvf::Color3f( cvf::Color3::WHITE ), 0.9f );
        QwtPlotItem* backgroundShape =
            createColumnShape( startX, endX, startDepth, endDepth, semiTransparentWhite, Qt::SolidPattern );
        m_combinedComponentGroup.addPlotItem( backgroundShape );

        QwtPlotItem* patternShape = createColumnShape( startX, endX, startDepth, endDepth, baseColor, brushStyle );
        m_combinedComponentGroup.addPlotItem( patternShape );
        if ( endX >= 0.0 )
        {
            QwtPlotItem* legendBGShape = createColumnShape( 0.0, 16.0, 0.0, 16.0, semiTransparentWhite, Qt::SolidPattern );
            m_combinedComponentGroup.addLegendItem( legendBGShape );

            QwtPlotItem* legendShape = createColumnShape( 0.0, 16.0, 0.0, 16.0, baseColor, brushStyle );
            m_combinedComponentGroup.addLegendItem( legendShape );
        }
    }
    else
    {
        QwtPlotItem* backgroundShape = createColumnShape( startX, endX, startDepth, endDepth, baseColor, Qt::SolidPattern );
        m_combinedComponentGroup.addPlotItem( backgroundShape );

        if ( endX >= 0.0 )
        {
            QwtPlotItem* legendShape = createColumnShape( 0.0, 16.0, 0.0, 16.0, baseColor, Qt::SolidPattern );
            m_combinedComponentGroup.addLegendItem( legendShape );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QwtPlotItem* RiuWellPathComponentPlotItem::createColumnShape( double         startX,
                                                              double         endX,
                                                              double         startDepth,
                                                              double         endDepth,
                                                              cvf::Color4f   baseColor,
                                                              Qt::BrushStyle brushStyle )
{
    QwtPlotShapeItem* columnShape = new QwtPlotShapeItem( label() );
    QPolygonF         polygon;
    QColor            color = RiaColorTools::toQColor( baseColor );

    polygon.push_back( QPointF( startX, startDepth ) );
    polygon.push_back( QPointF( endX, startDepth ) );
    polygon.push_back( QPointF( endX, endDepth ) );
    polygon.push_back( QPointF( startX, endDepth ) );
    polygon.push_back( QPointF( startX, startDepth ) );
    columnShape->setPolygon( polygon );
    columnShape->setXAxis( QwtPlot::xBottom );
    columnShape->setBrush( QBrush( color, brushStyle ) );
    columnShape->setLegendMode( QwtPlotShapeItem::LegendShape );
    columnShape->setLegendIconSize( QSize( 16, 16 ) );
    return columnShape;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color4f RiuWellPathComponentPlotItem::componentColor( float alpha /*= 1.0*/ ) const
{
    return cvf::Color4f( RiaColorTables::wellPathComponentColors()[m_componentType], alpha );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuWellPathComponentPlotItem::xValueRange( double* minimumValue, double* maximumValue ) const
{
    CVF_ASSERT( minimumValue && maximumValue );
    *maximumValue = 1.0;
    *minimumValue = -1.0;
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuWellPathComponentPlotItem::yValueRange( double* minimumValue, double* maximumValue ) const
{
    CVF_ASSERT( minimumValue && maximumValue );

    if ( minimumValue && maximumValue )
    {
        std::tie( *minimumValue, *maximumValue ) = depthsOfDepthType();
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellPathComponentPlotItem::setShowLabel( bool showLabel )
{
    m_showLabel = showLabel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellPathComponentPlotItem::setDepthType( RimWellLogPlot::DepthTypeEnum depthType )
{
    m_depthType = depthType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellPathComponentPlotItem::setContributeToLegend( bool contributeToLegend )
{
    m_combinedComponentGroup.setItemAttribute( QwtPlotItem::Legend, contributeToLegend );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellPathComponentPlotItem::setParentQwtPlotAndReplot( QwtPlot* plot )
{
    setParentQwtPlotNoReplot( plot );
    if ( m_parentQwtPlot )
    {
        m_parentQwtPlot->replot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellPathComponentPlotItem::setParentQwtPlotNoReplot( QwtPlot* plot )
{
    m_parentQwtPlot = plot;
    attachToQwt();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellPathComponentPlotItem::attachToQwt()
{
    if ( m_parentQwtPlot )
    {
        m_combinedComponentGroup.attach( m_parentQwtPlot );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellPathComponentPlotItem::detachFromQwt()
{
    m_combinedComponentGroup.detach();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellPathComponentPlotItem::reattachToQwt()
{
    detachFromQwt();
    attachToQwt();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuWellPathComponentPlotItem::legendTitle() const
{
    return m_legendTitle;
}
