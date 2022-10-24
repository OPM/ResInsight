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
#include "RiaPlotDefines.h"

#include "RimDepthTrackPlot.h"
#include "RimFishbones.h"
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
#include "RiuPlotAxis.h"
#include "RiuQwtPlotTools.h"

#include "qwt_plot.h"
#include "qwt_plot_marker.h"
#include "qwt_plot_shapeitem.h"
#include "qwt_text.h"

#include <QBrush>
#include <Qt>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWellPathComponentPlotItem::RiuWellPathComponentPlotItem( const RimWellPath* wellPath )
    : m_wellPath( wellPath )
    , m_componentType( RiaDefines::WellPathComponentType::WELL_PATH )
    , m_columnOffset( 0.0 )
    , m_depthType( RiaDefines::DepthTypeEnum::MEASURED_DEPTH )
    , m_depthOrientation( RiaDefines::Orientation::VERTICAL )
    , m_maxColumnOffset( 0.0 )
    , m_showLabel( false )
{
    CVF_ASSERT( wellPath && wellPath->wellPathGeometry() );
    m_startMD     = wellPath->startMD();
    m_endMD       = wellPath->endMD();
    m_label       = wellPath->name();
    m_legendTitle = "Well Tube";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWellPathComponentPlotItem::RiuWellPathComponentPlotItem( const RimWellPath*                   wellPath,
                                                            const RimWellPathComponentInterface* component )
    : m_wellPath( wellPath )
    , m_columnOffset( 0.0 )
    , m_depthType( RiaDefines::DepthTypeEnum::MEASURED_DEPTH )
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
        if ( otherAttribute->componentType() == RiaDefines::WellPathComponentType::CASING )
        {
            uniqueCasingDiameters.insert( otherAttribute->diameterInInches() );
        }
    }

    if ( !uniqueCasingDiameters.empty() )
    {
        m_maxColumnOffset = ( uniqueCasingDiameters.size() - 1 ) * 0.25;

        const RimWellPathAttribute* myAttribute = dynamic_cast<const RimWellPathAttribute*>( component );
        if ( myAttribute && myAttribute->componentType() == RiaDefines::WellPathComponentType::CASING )
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

    if ( m_componentType == RiaDefines::WellPathComponentType::WELL_PATH )
    {
        addColumnFeature( -0.25, 0.25, startDepth, endDepth, componentColor() );
    }
    else if ( m_componentType == RiaDefines::WellPathComponentType::CASING )
    {
        double posMin = 0.5 + m_columnOffset;
        double posMax = 0.75 + m_columnOffset;
        addColumnFeature( -posMax, -posMin, startDepth, endDepth, componentColor() );
        addColumnFeature( posMin, posMax, startDepth, endDepth, componentColor() );

        if ( m_depthOrientation == RiaDefines::Orientation::VERTICAL )
        {
            addMarker( -posMax, endDepth, 12, RiuPlotCurveSymbol::SYMBOL_LEFT_ANGLED_TRIANGLE, componentColor() );
            addMarker( posMax, endDepth, 12, RiuPlotCurveSymbol::SYMBOL_RIGHT_ANGLED_TRIANGLE, componentColor() );
        }
        else
        {
            addMarker( -posMax, endDepth, 12, RiuPlotCurveSymbol::SYMBOL_DOWN_ALIGNED_TRIANGLE, componentColor() );
            addMarker( posMax, endDepth, 12, RiuPlotCurveSymbol::SYMBOL_LEFT_ANGLED_TRIANGLE, componentColor() );
        }

        addMarker( casingTrackEnd,
                   endDepth,
                   12,
                   RiuPlotCurveSymbol::SYMBOL_RIGHT_ANGLED_TRIANGLE,
                   componentColor( 0.0 ),
                   label() );
    }
    else if ( m_componentType == RiaDefines::WellPathComponentType::LINER )
    {
        addColumnFeature( -0.5, -0.25, startDepth, endDepth, componentColor() );
        addColumnFeature( 0.25, 0.5, startDepth, endDepth, componentColor() );
        addMarker( casingTrackEnd,
                   endDepth,
                   10,
                   RiuPlotCurveSymbol::SYMBOL_RIGHT_ANGLED_TRIANGLE,
                   componentColor( 0.0 ),
                   label() );
    }
    else if ( m_componentType == RiaDefines::WellPathComponentType::PERFORATION_INTERVAL )
    {
        addColumnFeature( -casingTrackEnd, -0.25, startDepth, endDepth, componentColor(), Qt::Dense6Pattern );
        addColumnFeature( 0.25, casingTrackEnd, startDepth, endDepth, componentColor(), Qt::Dense6Pattern );
        // Empirically a spacing of around 30 in depth between symbols looks good in the most relevant zoom levels.
        const double markerSpacing = 30;
        const int    markerSize    = 6;
        double       markerDepth   = startDepth;

        auto plotSymbol1 = RiuPlotCurveSymbol::SYMBOL_LEFT_ALIGNED_TRIANGLE;
        auto plotSymbol2 = RiuPlotCurveSymbol::SYMBOL_RIGHT_ALIGNED_TRIANGLE;

        if ( m_depthOrientation == RiaDefines::Orientation::HORIZONTAL )
        {
            plotSymbol1 = RiuPlotCurveSymbol::SYMBOL_DOWN_TRIANGLE;
            plotSymbol2 = RiuPlotCurveSymbol::SYMBOL_UP_TRIANGLE;
        }

        while ( markerDepth < endDepth - 5 )
        {
            addMarker( -casingTrackEnd, markerDepth, markerSize, plotSymbol1, componentColor() );
            addMarker( casingTrackEnd, markerDepth, markerSize, plotSymbol2, componentColor() );

            markerDepth += markerSpacing;
        }
        addMarker( casingTrackEnd,
                   midDepth,
                   10,
                   RiuPlotCurveSymbol::SYMBOL_RIGHT_ALIGNED_TRIANGLE,
                   componentColor( 0.0 ),
                   label() );

        QwtPlotItem* legendItem1 =
            createMarker( 16.0, 0.0, 6, RiuPlotCurveSymbol::SYMBOL_RIGHT_ALIGNED_TRIANGLE, componentColor() );
        legendItem1->setLegendIconSize( QSize( 4, 8 ) );
        QwtPlotItem* legendItem2 =
            createMarker( 16.0, 8.0, 6, RiuPlotCurveSymbol::SYMBOL_RIGHT_ALIGNED_TRIANGLE, componentColor() );
        legendItem2->setLegendIconSize( QSize( 4, 8 ) );
        m_combinedComponentGroup.addLegendItem( legendItem1 );
        m_combinedComponentGroup.addLegendItem( legendItem2 );
    }
    else if ( m_componentType == RiaDefines::WellPathComponentType::FISHBONES )
    {
        addColumnFeature( -casingTrackEnd, -0.25, startDepth, endDepth, componentColor(), Qt::BDiagPattern );
        addColumnFeature( 0.25, casingTrackEnd, startDepth, endDepth, componentColor(), Qt::FDiagPattern );
        addMarker( casingTrackEnd,
                   midDepth,
                   10,
                   RiuPlotCurveSymbol::SYMBOL_RIGHT_ANGLED_TRIANGLE,
                   componentColor( 0.0 ),
                   label() );
    }
    else if ( m_componentType == RiaDefines::WellPathComponentType::FRACTURE )
    {
        addColumnFeature( -casingTrackEnd, -0.25, startDepth, endDepth, componentColor(), Qt::SolidPattern );
        addColumnFeature( 0.25, casingTrackEnd, startDepth, endDepth, componentColor(), Qt::SolidPattern );
        addMarker( casingTrackEnd,
                   startDepth,
                   10,
                   RiuPlotCurveSymbol::SYMBOL_NONE,
                   componentColor(),
                   "",
                   Qt::AlignTop | Qt::AlignRight,
                   Qt::Horizontal,
                   true );
        addMarker( casingTrackEnd,
                   endDepth,
                   10,
                   RiuPlotCurveSymbol::SYMBOL_NONE,
                   componentColor(),
                   "",
                   Qt::AlignTop | Qt::AlignRight,
                   Qt::Horizontal,
                   true );
        addMarker( casingTrackEnd,
                   startDepth,
                   1,
                   RiuPlotCurveSymbol::SYMBOL_RIGHT_ANGLED_TRIANGLE,
                   componentColor( 0.0f ),
                   label(),
                   Qt::AlignTop | Qt::AlignRight );
    }
    else if ( m_componentType == RiaDefines::WellPathComponentType::ICD )
    {
        for ( double md : m_subMDs )
        {
            addMarker( 0.0,
                       md,
                       16,
                       RiuPlotCurveSymbol::SYMBOL_ELLIPSE,
                       componentColor(),
                       "",
                       Qt::AlignCenter,
                       Qt::Horizontal,
                       false,
                       true );
        }
        m_combinedComponentGroup.addLegendItem(
            createMarker( 0.0, 0.0, 12.0, RiuPlotCurveSymbol::SYMBOL_ELLIPSE, componentColor() ) );
    }
    else if ( m_componentType == RiaDefines::WellPathComponentType::ICV )
    {
        for ( double md : m_subMDs )
        {
            addMarker( 0.0,
                       md,
                       16,
                       RiuPlotCurveSymbol::SYMBOL_ELLIPSE,
                       componentColor(),
                       "",
                       Qt::AlignCenter,
                       Qt::Horizontal,
                       false,
                       true );
        }
        m_combinedComponentGroup.addLegendItem(
            createMarker( 0.0, 0.0, 12.0, RiuPlotCurveSymbol::SYMBOL_ELLIPSE, componentColor() ) );
    }
    else if ( m_componentType == RiaDefines::WellPathComponentType::AICD )
    {
        for ( double md : m_subMDs )
        {
            addMarker( 0.0,
                       md,
                       16,
                       RiuPlotCurveSymbol::SYMBOL_ELLIPSE,
                       componentColor(),
                       "",
                       Qt::AlignCenter,
                       Qt::Horizontal,
                       false,
                       true );
        }
        m_combinedComponentGroup.addLegendItem(
            createMarker( 0.0, 0.0, 12.0, RiuPlotCurveSymbol::SYMBOL_ELLIPSE, componentColor() ) );
    }
    else if ( m_componentType == RiaDefines::WellPathComponentType::PACKER )
    {
        addColumnFeature( -1.1 * casingTrackEnd, -0.25, startDepth, endDepth, componentColor(), Qt::DiagCrossPattern );
        addColumnFeature( 0.25, 1.1 * casingTrackEnd, startDepth, endDepth, componentColor(), Qt::DiagCrossPattern );
        addMarker( casingTrackEnd,
                   midDepth,
                   10,
                   RiuPlotCurveSymbol::SYMBOL_RIGHT_ANGLED_TRIANGLE,
                   componentColor( 0.0 ),
                   label() );
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

    if ( m_depthType == RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH ||
         m_depthType == RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH_RKB )
    {
        endDepth       = -m_wellPath->wellPathGeometry()->interpolatedPointAlongWellPath( m_endMD ).z();
        double rkbDiff = m_wellPath->wellPathGeometry()->rkbDiff();
        if ( rkbDiff == std::numeric_limits<double>::infinity() )
        {
            rkbDiff = 0.0;
        }

        if ( m_componentType == RiaDefines::WellPathComponentType::WELL_PATH )
        {
            startDepth = 0.0;
            if ( m_depthType == RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH )
            {
                startDepth -= rkbDiff;
            }
            else if ( m_depthType == RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH_RKB )
            {
                endDepth += m_wellPath->wellPathGeometry()->rkbDiff();
            }
        }
        else
        {
            startDepth = -m_wellPath->wellPathGeometry()->interpolatedPointAlongWellPath( m_startMD ).z();
            if ( m_depthType == RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH_RKB )
            {
                startDepth += m_wellPath->wellPathGeometry()->rkbDiff();
                endDepth += m_wellPath->wellPathGeometry()->rkbDiff();
            }
        }
    }
    return std::make_pair( startDepth, endDepth );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellPathComponentPlotItem::addMarker( double                              position,
                                              double                              depth,
                                              int                                 size,
                                              RiuPlotCurveSymbol::PointSymbolEnum symbolType,
                                              cvf::Color4f                        baseColor,
                                              const QString&                      label /*= QString( "" )*/,
                                              Qt::Alignment   labelAlignment /*= Qt::AlignVCenter | Qt::AlignRight*/,
                                              Qt::Orientation labelOrientation /*= Qt::Horizontal*/,
                                              bool            drawLine /*= false*/,
                                              bool            contrastTextColor /*= false */ )
{
    QwtPlotItem* marker =
        createMarker( position, depth, size, symbolType, baseColor, label, labelAlignment, labelOrientation, drawLine, contrastTextColor );
    m_combinedComponentGroup.addPlotItem( marker );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QwtPlotItem*
    RiuWellPathComponentPlotItem::createMarker( double                              position,
                                                double                              depth,
                                                int                                 size,
                                                RiuPlotCurveSymbol::PointSymbolEnum symbolType,
                                                cvf::Color4f                        baseColor,
                                                const QString&                      label /*= QString( "" )*/,
                                                Qt::Alignment   labelAlignment /*= Qt::AlignVCenter | Qt::AlignRight*/,
                                                Qt::Orientation labelOrientation /*= Qt::Horizontal*/,
                                                bool            drawLine /*= false*/,
                                                bool            contrastTextColor /*= false */ )
{
    QColor bgColor   = RiaColorTools::toQColor( baseColor );
    QColor textColor = RiaColorTools::toQColor( baseColor.toColor3f(), 1.0 );
    if ( contrastTextColor )
    {
        textColor = RiaColorTools::toQColor( RiaColorTools::contrastColor( baseColor.toColor3f() ) );
    }
    QwtPlotMarker* marker = new QwtPlotMarker( label );
    RiuQwtSymbol*  symbol = new RiuQwtSymbol( symbolType, "", RiuPlotCurveSymbol::LabelRightOfSymbol );
    symbol->setSize( size, size );
    symbol->setColor( bgColor );
    marker->setSymbol( symbol );
    marker->setSpacing( 6 );

    if ( m_depthOrientation == RiaDefines::Orientation::HORIZONTAL )
    {
        marker->setXValue( depth );
        marker->setYValue( position );
        labelOrientation = Qt::Vertical;
        labelAlignment   = Qt::AlignTop;
    }
    else
    {
        marker->setXValue( position );
        marker->setYValue( depth );
    }

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
        if ( m_depthOrientation == RiaDefines::Orientation::HORIZONTAL )
        {
            marker->setLineStyle( QwtPlotMarker::HLine );
        }
        else
        {
            marker->setLineStyle( QwtPlotMarker::VLine );
        }

        marker->setLinePen( bgColor, 2.0, Qt::SolidLine );
    }
    return marker;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellPathComponentPlotItem::addColumnFeature( double         startPosition,
                                                     double         endPosition,
                                                     double         startDepth,
                                                     double         endDepth,
                                                     cvf::Color4f   baseColor,
                                                     Qt::BrushStyle brushStyle /*= Qt::SolidPattern */ )
{
    double startX = startPosition;
    double endX   = endPosition;
    double startY = startDepth;
    double endY   = endDepth;

    if ( m_depthOrientation == RiaDefines::Orientation::HORIZONTAL )
    {
        startX = startDepth;
        endX   = endDepth;
        startY = startPosition;
        endY   = endPosition;
    }

    QColor baseQColor = RiaColorTools::toQColor( baseColor );
    if ( brushStyle != Qt::SolidPattern )
    {
        // If we're doing a special pattern, draw the background in white first over the existing pattern
        QColor semiTransparentWhite( Qt::white );
        semiTransparentWhite.setAlphaF( 0.9f );
        QwtPlotItem* backgroundShape =
            RiuQwtPlotTools::createBoxShape( label(), startX, endX, startY, endY, semiTransparentWhite, Qt::SolidPattern );
        m_combinedComponentGroup.addPlotItem( backgroundShape );

        QwtPlotItem* patternShape =
            RiuQwtPlotTools::createBoxShape( label(), startX, endX, startY, endY, baseQColor, brushStyle );
        m_combinedComponentGroup.addPlotItem( patternShape );
        if ( endX >= 0.0 )
        {
            QwtPlotItem* legendBGShape =
                RiuQwtPlotTools::createBoxShape( label(), 0.0, 16.0, 0.0, 16.0, semiTransparentWhite, Qt::SolidPattern );
            m_combinedComponentGroup.addLegendItem( legendBGShape );

            QwtPlotItem* legendShape =
                RiuQwtPlotTools::createBoxShape( label(), 0.0, 16.0, 0.0, 16.0, baseQColor, brushStyle );
            m_combinedComponentGroup.addLegendItem( legendShape );
        }
    }
    else
    {
        QwtPlotItem* backgroundShape =
            RiuQwtPlotTools::createBoxShape( label(), startX, endX, startY, endY, baseQColor, Qt::SolidPattern );
        m_combinedComponentGroup.addPlotItem( backgroundShape );

        if ( endX >= 0.0 )
        {
            QwtPlotItem* legendShape =
                RiuQwtPlotTools::createBoxShape( label(), 0.0, 16.0, 0.0, 16.0, baseQColor, Qt::SolidPattern );
            m_combinedComponentGroup.addLegendItem( legendShape );
        }
    }
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
bool RiuWellPathComponentPlotItem::propertyValueRange( double* minimumValue, double* maximumValue ) const
{
    CVF_ASSERT( minimumValue && maximumValue );
    *maximumValue = 1.0;
    *minimumValue = -1.0;
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuWellPathComponentPlotItem::depthValueRange( double* minimumValue, double* maximumValue ) const
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
void RiuWellPathComponentPlotItem::setDepthOrientation( RiaDefines::Orientation depthOrientation )
{
    m_depthOrientation = depthOrientation;
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
    setParentPlotNoReplot( plot );
    if ( m_parentQwtPlot )
    {
        m_parentQwtPlot->replot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellPathComponentPlotItem::setParentPlotNoReplot( QwtPlot* plot )
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

        auto riuAxis = RimDepthTrackPlot::annotationAxis( m_depthOrientation );
        auto qwtAxis = RiuQwtPlotTools::toQwtPlotAxisEnum( riuAxis.axis() );

        if ( m_depthOrientation == RiaDefines::Orientation::VERTICAL )
        {
            m_combinedComponentGroup.setXAxis( qwtAxis );
        }
        else
        {
            m_combinedComponentGroup.setYAxis( qwtAxis );
        }
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
