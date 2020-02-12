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

#include "RiuMohrsCirclePlot.h"

#include "RiaColorTables.h"

#include "Riu3dSelectionManager.h"
#include "RiuQwtPlotTools.h"

#include "RigFemPart.h"
#include "RigFemPartCollection.h"
#include "RigFemPartGrid.h"
#include "RigFemPartResultsCollection.h"
#include "RigFemResultPosEnum.h"
#include "RigGeoMechCaseData.h"

#include "Rim2dIntersectionView.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechResultDefinition.h"
#include "RimGeoMechView.h"

#include "cvfAssert.h"

#include <QPainterPath>
#include <QTimer>
#include <QWidget>
#include <qevent.h>

#include "qwt_legend.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_rescaler.h"
#include "qwt_plot_shapeitem.h"

#include <cmath>

//==================================================================================================
///
/// \class RiuMohrsCirclePlot
///
///
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuMohrsCirclePlot::RiuMohrsCirclePlot( QWidget* parent )
    : RiuDockedQwtPlot( parent )
    , m_viewToFollowAnimationFrom( nullptr )
    , m_scheduleUpdateAxisScaleTimer( nullptr )
{
    RiuQwtPlotTools::setCommonPlotBehaviour( this );

    enableAxis( QwtPlot::xBottom, true );
    enableAxis( QwtPlot::yLeft, true );
    enableAxis( QwtPlot::xTop, false );
    enableAxis( QwtPlot::yRight, false );

    setAxisTitle( QwtPlot::xBottom, "Effective Normal Stress" );
    setAxisTitle( QwtPlot::yLeft, "Shear Stress" );

    applyFontSizes( false );

    // The legend will be deleted in the destructor of the plot or when
    // another legend is inserted.
    QwtLegend* legend = new QwtLegend( this );
    this->insertLegend( legend, BottomLegend );

    // this->setTitle(QString("SE"));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuMohrsCirclePlot::~RiuMohrsCirclePlot()
{
    deletePlotItems();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMohrsCirclePlot::appendSelection( const RiuSelectionItem* selectionItem )
{
    m_viewToFollowAnimationFrom = nullptr;

    if ( this->isVisible() )
    {
        Rim3dView*               newFollowAnimView    = nullptr;
        RiuGeoMechSelectionItem* geoMechSelectionItem = nullptr;

        geoMechSelectionItem = extractGeoMechSelectionItem( selectionItem, newFollowAnimView );

        if ( geoMechSelectionItem )
        {
            const size_t       gridIndex = geoMechSelectionItem->m_gridIndex;
            const size_t       cellIndex = geoMechSelectionItem->m_cellIndex;
            const cvf::Color3f color     = geoMechSelectionItem->m_color;

            addOrUpdateCurves( geoMechSelectionItem->m_resultDefinition,
                               geoMechSelectionItem->m_timestepIdx,
                               gridIndex,
                               cellIndex,
                               cvf::Color3ub( color ) );

            updatePlot();

            m_viewToFollowAnimationFrom = geoMechSelectionItem->m_view;
        }
    }
    else
    {
        this->clearPlot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMohrsCirclePlot::clearPlot()
{
    deletePlotItems();

    m_viewToFollowAnimationFrom = nullptr;

    this->replot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMohrsCirclePlot::updateOnTimeStepChanged( Rim3dView* changedView )
{
    if ( !this->isVisible() )
    {
        return;
    }

    // Don't update the plot if the view that changed time step is different
    // from the view that was the source of the current plot

    if ( changedView != m_viewToFollowAnimationFrom )
    {
        return;
    }

    std::vector<MohrsCirclesInfo> mohrsCiclesInfosCopy = m_mohrsCiclesInfos;
    deletePlotItems();

    for ( const MohrsCirclesInfo& mohrInfo : mohrsCiclesInfosCopy )
    {
        addOrUpdateCurves( mohrInfo.geomResDef,
                           changedView->currentTimeStep(),
                           mohrInfo.gridIndex,
                           mohrInfo.elmIndex,
                           mohrInfo.color );
    }

    updatePlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuMohrsCirclePlot::sizeHint() const
{
    return QSize( 100, 100 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuMohrsCirclePlot::minimumSizeHint() const
{
    return QSize( 0, 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMohrsCirclePlot::addOrUpdateMohrCircleCurves( const MohrsCirclesInfo& mohrsCirclesInfo )
{
    const cvf::Vec3f& principals = mohrsCirclesInfo.principals;

    std::array<std::pair<double /*radius*/, double /*centerX*/>, 3> mohrsCircles;

    mohrsCircles[0].first  = ( principals[0] - principals[2] ) / 2.0;
    mohrsCircles[0].second = ( principals[0] + principals[2] ) / 2.0;

    mohrsCircles[1].first  = ( principals[1] - principals[2] ) / 2.0;
    mohrsCircles[1].second = ( principals[1] + principals[2] ) / 2.0;

    mohrsCircles[2].first  = ( principals[0] - principals[1] ) / 2.0;
    mohrsCircles[2].second = ( principals[0] + principals[1] ) / 2.0;

    for ( size_t i = 0; i < 3; i++ )
    {
        QwtPlotShapeItem* plotItem = new QwtPlotShapeItem( "Circle" );

        QPainterPath* circleDrawing = new QPainterPath();
        QPointF       center( mohrsCircles[i].second, 0 );
        circleDrawing->addEllipse( center, mohrsCircles[i].first, mohrsCircles[i].first );

        plotItem->setPen( QColor( mohrsCirclesInfo.color.r(), mohrsCirclesInfo.color.g(), mohrsCirclesInfo.color.b() ) );
        plotItem->setShape( *circleDrawing );
        plotItem->setRenderHint( QwtPlotItem::RenderAntialiased, true );

        if ( i == 0 )
        {
            QString textBuilder;
            textBuilder.append(
                QString( "<b>FOS</b>: %1, " ).arg( QString::number( mohrsCirclesInfo.factorOfSafety, 'f', 2 ) ) );

            textBuilder.append( QString( "<b>Element Id</b>: %1, <b>ijk</b>[%2, %3, %4]," )
                                    .arg( mohrsCirclesInfo.elmIndex )
                                    .arg( mohrsCirclesInfo.i )
                                    .arg( mohrsCirclesInfo.j )
                                    .arg( mohrsCirclesInfo.k ) );

            textBuilder.append( QString( "<b>&sigma;<sub>1</sub></b>: %1, " ).arg( principals[0] ) );
            textBuilder.append( QString( "<b>&sigma;<sub>2</sub></b>: %1, " ).arg( principals[1] ) );
            textBuilder.append( QString( "<b>&sigma;<sub>3</sub></b>: %1" ).arg( principals[2] ) );

            plotItem->setTitle( textBuilder );
            plotItem->setItemAttribute( QwtPlotItem::Legend );
        }

        plotItem->attach( this );

        m_circlePlotItems.push_back( plotItem );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMohrsCirclePlot::deleteCircles()
{
    for ( size_t i = 0; i < m_circlePlotItems.size(); i++ )
    {
        m_circlePlotItems[i]->detach();
        delete m_circlePlotItems[i];
    }

    m_circlePlotItems.clear();

    for ( size_t i = 0; i < m_transparentCurves.size(); i++ )
    {
        m_transparentCurves[i]->detach();
        delete m_transparentCurves[i];
    }

    m_transparentCurves.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMohrsCirclePlot::addorUpdateEnvelopeCurve( const cvf::Vec3f& principals, const RimGeoMechCase* geomCase )
{
    if ( !geomCase ) return;

    double cohesion      = geomCase->cohesion();
    double frictionAngle = geomCase->frictionAngleDeg();

    if ( cohesion == HUGE_VAL || frictionAngle == HUGE_VAL || frictionAngle >= 90 )
    {
        return;
    }

    double xVals[2];
    double yVals[2];

    double tanFrictionAngle = cvf::Math::abs( cvf::Math::tan( cvf::Math::toRadians( frictionAngle ) ) );

    if ( tanFrictionAngle == 0 || tanFrictionAngle == HUGE_VAL )
    {
        return;
    }

    double x = cohesion / tanFrictionAngle;

    xVals[0] = -x;
    xVals[1] = principals[0];

    yVals[0] = 0;
    yVals[1] = ( cohesion / x ) * ( x + principals[0] );

    // If envelope for the view already exists, check if a "larger" envelope should be created
    if ( m_envolopePlotItems.find( geomCase ) != m_envolopePlotItems.end() )
    {
        if ( yVals[1] <= m_envolopePlotItems[geomCase]->maxYValue() )
        {
            return;
        }
        else
        {
            m_envolopePlotItems[geomCase]->detach();
            delete m_envolopePlotItems[geomCase];
            m_envolopePlotItems.erase( geomCase );
        }
    }

    QwtPlotCurve* qwtCurve = new QwtPlotCurve();

    qwtCurve->setSamples( xVals, yVals, 2 );

    qwtCurve->setStyle( QwtPlotCurve::Lines );
    qwtCurve->setRenderHint( QwtPlotItem::RenderAntialiased, true );

    const QPen curvePen( envelopeColor( geomCase ) );
    qwtCurve->setPen( curvePen );

    qwtCurve->setTitle( QString( "<b>Envelope for %1</b>, (<b>S<sub>0</sub></b>: %2, <b>&Phi;</b>: %3)" )
                            .arg( geomCase->caseUserDescription() )
                            .arg( cohesion )
                            .arg( frictionAngle ) );

    qwtCurve->attach( this );

    m_envolopePlotItems[geomCase] = qwtCurve;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMohrsCirclePlot::deleteEnvelopes()
{
    for ( const std::pair<const RimGeoMechCase*, QwtPlotCurve*>& envelope : m_envolopePlotItems )
    {
        envelope.second->detach();
        delete envelope.second;
    }

    m_envolopePlotItems.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuMohrsCirclePlot::addOrUpdateCurves( const RimGeoMechResultDefinition* geomResDef,
                                            size_t                            timeStepIndex,
                                            size_t                            gridIndex,
                                            size_t                            elmIndex,
                                            const cvf::Color3ub&              color )
{
    RigFemPart* femPart = geomResDef->ownerCaseData()->femParts()->part( gridIndex );

    if ( femPart->elementType( elmIndex ) != HEX8P ) return false;

    RigFemPartResultsCollection* resultCollection = geomResDef->geoMechCase()->geoMechData()->femPartResults();

    RigFemResultAddress address( RigFemResultPosEnum::RIG_ELEMENT_NODAL, "SE", "" );

    // TODO: All tensors are calculated every time this function is called. FIX

    std::vector<caf::Ten3f> vertexTensors = resultCollection->tensors( address, 0, (int)timeStepIndex );
    if ( vertexTensors.empty() )
    {
        return false;
    }

    // Calculate average tensor in element
    caf::Ten3f tensorSumOfElmNodes = vertexTensors[femPart->elementNodeResultIdx( (int)elmIndex, 0 )];
    for ( int i = 1; i < 8; i++ )
    {
        tensorSumOfElmNodes = tensorSumOfElmNodes + vertexTensors[femPart->elementNodeResultIdx( (int)elmIndex, i )];
    }
    caf::Ten3f elmTensor = tensorSumOfElmNodes * ( 1.0 / 8.0 );

    cvf::Vec3f principals = elmTensor.calculatePrincipals( nullptr );

    if ( !isValidPrincipals( principals ) )
    {
        return false;
    }

    double cohesion         = geomResDef->geoMechCase()->cohesion();
    double frictionAngleDeg = geomResDef->geoMechCase()->frictionAngleDeg();

    size_t i, j, k;
    bool   validIndex = femPart->getOrCreateStructGrid()->ijkFromCellIndex( elmIndex, &i, &j, &k );

    CVF_ASSERT( validIndex );
    if ( validIndex )
    {
        MohrsCirclesInfo mohrsCircle( principals,
                                      gridIndex,
                                      elmIndex,
                                      i,
                                      j,
                                      k,
                                      geomResDef,
                                      calculateFOS( principals, frictionAngleDeg, cohesion ),
                                      color );

        m_mohrsCiclesInfos.push_back( mohrsCircle );

        addorUpdateEnvelopeCurve( mohrsCircle.principals, mohrsCircle.geomResDef->geoMechCase() );
        addOrUpdateMohrCircleCurves( mohrsCircle );
        updateTransparentCurvesOnPrincipals();

        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMohrsCirclePlot::updatePlot()
{
    setAxesScaleAndReplot();
    // Update axis scale is called one more time because the legend which is added on a later stage may disrupt the canvas
    scheduleUpdateAxisScale();
}

//--------------------------------------------------------------------------------------------------
/// Add a transparent curve to make tooltip available on principals crossing the x-axis
//--------------------------------------------------------------------------------------------------
void RiuMohrsCirclePlot::updateTransparentCurvesOnPrincipals()
{
    for ( size_t i = 0; i < m_transparentCurves.size(); i++ )
    {
        m_transparentCurves[i]->detach();
        delete m_transparentCurves[i];
    }

    m_transparentCurves.clear();

    for ( const MohrsCirclesInfo& mohrCircleInfo : m_mohrsCiclesInfos )
    {
        QwtPlotCurve* transparentCurve = new QwtPlotCurve();

        QVector<QPointF> qVectorPoints;

        qVectorPoints.push_back( QPointF( mohrCircleInfo.principals[0], 0 ) );
        qVectorPoints.push_back( QPointF( mohrCircleInfo.principals[1], 0 ) );
        qVectorPoints.push_back( QPointF( mohrCircleInfo.principals[2], 0 ) );

        transparentCurve->setSamples( qVectorPoints );
        transparentCurve->setYAxis( QwtPlot::yLeft );
        transparentCurve->setStyle( QwtPlotCurve::NoCurve );
        transparentCurve->setLegendAttribute( QwtPlotCurve::LegendNoAttribute );

        transparentCurve->attach( this );
        m_transparentCurves.push_back( transparentCurve );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiuMohrsCirclePlot::largestCircleRadiusInPlot() const
{
    double currentLargestDiameter = -HUGE_VAL;

    for ( const MohrsCirclesInfo& mohrCircleInfo : m_mohrsCiclesInfos )
    {
        if ( mohrCircleInfo.principals[0] > currentLargestDiameter )
        {
            currentLargestDiameter = mohrCircleInfo.principals[0] - mohrCircleInfo.principals[2];
        }
    }

    return currentLargestDiameter / 2;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiuMohrsCirclePlot::smallestPrincipal() const
{
    double currentSmallestPrincipal = HUGE_VAL;

    for ( const MohrsCirclesInfo& mohrCircleInfo : m_mohrsCiclesInfos )
    {
        if ( mohrCircleInfo.principals[2] < currentSmallestPrincipal )
        {
            currentSmallestPrincipal = mohrCircleInfo.principals[2];
        }
    }

    return currentSmallestPrincipal;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiuMohrsCirclePlot::largestPrincipal() const
{
    double currentLargestPrincipal = -HUGE_VAL;

    for ( const MohrsCirclesInfo& mohrCircleInfo : m_mohrsCiclesInfos )
    {
        if ( mohrCircleInfo.principals[0] > currentLargestPrincipal )
        {
            currentLargestPrincipal = mohrCircleInfo.principals[0];
        }
    }

    return currentLargestPrincipal;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuMohrsCirclePlot::isValidPrincipals( const cvf::Vec3f& principals )
{
    float p1 = principals[0];
    float p2 = principals[1];
    float p3 = principals[2];

    // Inf
    if ( p1 == HUGE_VAL || p2 == HUGE_VAL || p3 == HUGE_VAL )
    {
        return false;
    }

    // Nan
    if ( ( p1 != p1 ) || ( p2 != p2 ) || p3 != p3 )
    {
        return false;
    }

    // Principal rules:
    if ( ( p1 < p2 ) || ( p2 < p3 ) )
    {
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
float RiuMohrsCirclePlot::calculateFOS( const cvf::Vec3f& principals, double frictionAngle, double cohesion )
{
    if ( cvf::Math::cos( frictionAngle ) == 0 )
    {
        return std::nan( "" );
    }

    float se1 = principals[0];
    float se3 = principals[2];

    float tanFricAng        = cvf::Math::tan( cvf::Math::toRadians( frictionAngle ) );
    float cohPrTanFricAngle = 1.0f * cohesion / tanFricAng;

    float dsm = RigFemPartResultsCollection::dsm( se1, se3, tanFricAng, cohPrTanFricAngle );

    return 1.0f / dsm;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QColor RiuMohrsCirclePlot::envelopeColor( const RimGeoMechCase* geomCase )
{
    if ( m_envolopeColors.find( geomCase ) == m_envolopeColors.end() )
    {
        cvf::Color3ub cvfColor =
            RiaColorTables::summaryCurveDefaultPaletteColors().cycledColor3ub( m_envolopeColors.size() );

        QColor color( cvfColor.r(), cvfColor.g(), cvfColor.b() );

        m_envolopeColors[geomCase] = color;
    }

    return m_envolopeColors[geomCase];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMohrsCirclePlot::deletePlotItems()
{
    m_mohrsCiclesInfos.clear();

    deleteCircles();
    deleteEnvelopes();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMohrsCirclePlot::scheduleUpdateAxisScale()
{
    if ( !m_scheduleUpdateAxisScaleTimer )
    {
        m_scheduleUpdateAxisScaleTimer = new QTimer( this );
        connect( m_scheduleUpdateAxisScaleTimer, SIGNAL( timeout() ), this, SLOT( setAxesScaleAndReplot() ) );
    }

    if ( !m_scheduleUpdateAxisScaleTimer->isActive() )
    {
        m_scheduleUpdateAxisScaleTimer->setSingleShot( true );
        m_scheduleUpdateAxisScaleTimer->start( 100 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMohrsCirclePlot::resizeEvent( QResizeEvent* e )
{
    setAxesScaleAndReplot();

    // Update axis scale is called one more time because setAxesScaleAndReplot does not work the first
    // time if the user does a very quick resizing of the window
    scheduleUpdateAxisScale();
    QwtPlot::resizeEvent( e );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMohrsCirclePlot::idealAxesEndPoints( double* xMin, double* xMax, double* yMax ) const
{
    *xMin = HUGE_VAL;
    *xMax = -HUGE_VAL;
    *yMax = -HUGE_VAL;

    double maxYEnvelope = -HUGE_VAL;
    for ( const std::pair<const RimGeoMechCase*, QwtPlotCurve*>& envelope : m_envolopePlotItems )
    {
        double tempMax = envelope.second->maxYValue();
        if ( tempMax > maxYEnvelope )
        {
            maxYEnvelope = tempMax;
        }
    }

    *yMax = std::max( maxYEnvelope, 1.2 * largestCircleRadiusInPlot() );

    double minXEvelope = HUGE_VAL;
    for ( const std::pair<const RimGeoMechCase*, QwtPlotCurve*>& envelope : m_envolopePlotItems )
    {
        double tempMin = envelope.second->minXValue();
        if ( tempMin < minXEvelope )
        {
            minXEvelope = tempMin;
        }
    }

    if ( minXEvelope < 0 )
    {
        *xMin = minXEvelope;
    }
    else
    {
        *xMin = 1.1 * smallestPrincipal();
    }

    *xMax = 1.1 * largestPrincipal();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMohrsCirclePlot::setAxesScaleAndReplot()
{
    // yMin is always 0
    double xMin, xMax, yMax;
    idealAxesEndPoints( &xMin, &xMax, &yMax );

    if ( xMax == -HUGE_VAL || xMin == HUGE_VAL || yMax == -HUGE_VAL )
    {
        return;
    }

    int canvasHeight = this->canvas()->height();
    int canvasWidth  = this->canvas()->width();

    const double minPlotWidth  = xMax - xMin;
    const double minPlotHeight = yMax;

    double xMaxDisplayed = xMax;
    double yMaxDisplayed = yMax;

    double canvasWidthOverHeightRatio = ( 1.0 * canvasWidth ) / ( 1.0 * canvasHeight );

    // widthToKeepAspectRatio increases when canvas height is increased
    double widthToKeepAspectRatio = minPlotHeight * canvasWidthOverHeightRatio;
    // heightToKeepAspectRatio increases when canvas width is increased
    double heightToKeepAspectRatio = minPlotWidth / canvasWidthOverHeightRatio;

    if ( widthToKeepAspectRatio > minPlotWidth )
    {
        xMaxDisplayed = widthToKeepAspectRatio + xMin;
    }
    else if ( heightToKeepAspectRatio > minPlotHeight )
    {
        yMaxDisplayed = heightToKeepAspectRatio;
    }

    this->setAxisScale( QwtPlot::yLeft, 0, yMaxDisplayed );
    this->setAxisScale( QwtPlot::xBottom, xMin, xMaxDisplayed );

    this->replot();
}

RiuGeoMechSelectionItem* RiuMohrsCirclePlot::extractGeoMechSelectionItem( const RiuSelectionItem* selectionItem,
                                                                          Rim3dView*&             newFollowAnimView )
{
    newFollowAnimView                             = nullptr;
    RiuGeoMechSelectionItem* geoMechSelectionItem = nullptr;

    geoMechSelectionItem = dynamic_cast<RiuGeoMechSelectionItem*>( const_cast<RiuSelectionItem*>( selectionItem ) );

    if ( geoMechSelectionItem )
    {
        // If we clicked in an geoMech view, and hit something using the standard result definition there,
        // set this up to follow the animation there.

        RimGeoMechView* clickedInEclView = dynamic_cast<RimGeoMechView*>( geoMechSelectionItem->m_view.p() );

        if ( clickedInEclView && clickedInEclView->cellResult() == geoMechSelectionItem->m_resultDefinition.p() )
        {
            newFollowAnimView = geoMechSelectionItem->m_view;
        }
    }
    else
    {
        auto intersectionSelItem = dynamic_cast<const Riu2dIntersectionSelectionItem*>( selectionItem );

        if ( intersectionSelItem && intersectionSelItem->geoMechSelectionItem() )
        {
            geoMechSelectionItem = intersectionSelItem->geoMechSelectionItem();
            newFollowAnimView    = intersectionSelItem->view();
        }
    }

    return geoMechSelectionItem;
}
