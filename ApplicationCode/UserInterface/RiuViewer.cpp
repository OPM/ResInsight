/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RiuViewer.h"

#include "RiaApplication.h"
#include "RiaBaseDefs.h"
#include "RiaColorTools.h"
#include "RiaPreferences.h"
#include "RiaRegressionTestRunner.h"

#include "RimCase.h"
#include "RimGridView.h"
#include "RimProject.h"
#include "RimViewController.h"
#include "RimViewLinker.h"

#include "RivGridBoxGenerator.h"
#include "RivTernarySaturationOverlayItem.h"
#include "WindowEdgeAxesOverlayItem/RivWindowEdgeAxesOverlayItem.h"

#include "RiuCadNavigation.h"
#include "RiuComparisonViewMover.h"
#include "RiuGeoQuestNavigation.h"
#include "RiuRmsNavigation.h"
#include "RiuSimpleHistogramWidget.h"
#include "RiuViewerCommands.h"

#include "cafPdmUiSelection3dEditorVisualizer.h"

#include "cafCategoryLegend.h"
#include "cafCeetronPlusNavigation.h"
#include "cafDisplayCoordTransform.h"
#include "cafEffectGenerator.h"
#include "cafFrameAnimationControl.h"
#include "cafOverlayScalarMapperLegend.h"
#include "cafOverlayScaleLegend.h"
#include "cafQStyledProgressBar.h"
#include "cafStyleSheetTools.h"
#include "cafTitledOverlayFrame.h"

#include "cvfCamera.h"
#include "cvfFont.h"
#include "cvfOpenGLResourceManager.h"
#include "cvfOverlayAxisCross.h"
#include "cvfOverlayItem.h"
#include "cvfPartRenderHintCollection.h"
#include "cvfRenderQueueSorter.h"
#include "cvfRenderSequence.h"
#include "cvfRendering.h"
#include "cvfScene.h"

#include <QLabel>
#include <QMouseEvent>

#include <algorithm>

using cvf::ManipulatorTrackball;

const double RI_MIN_NEARPLANE_DISTANCE = 0.1;

std::unique_ptr<QCursor> RiuViewer::s_hoverCursor;

//==================================================================================================
///
/// \class RiuViewer
/// \ingroup ResInsight
///
///
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuViewer::RiuViewer( const QGLFormat& format, QWidget* parent )
    : caf::Viewer( format, parent )
    , m_isNavigationRotationEnabled( true )
{
    cvf::Font* standardFont = RiaGuiApplication::instance()->defaultSceneFont();
    QFont      font         = RiaGuiApplication::instance()->font();
    font.setPointSize( RiaFontCache::pointSizeFromFontSizeEnum(
        RiaGuiApplication::instance()->preferences()->defaultSceneFontSize() ) );

    m_axisCross = new cvf::OverlayAxisCross( m_mainCamera.p(), standardFont );
    m_axisCross->setAxisLabels( "X", "Y", "Z" );
    m_axisCross->setLayout( cvf::OverlayItem::VERTICAL, cvf::OverlayItem::BOTTOM_RIGHT );
    overlayItemsRendering()->addOverlayItem( m_axisCross.p() );
    m_showAxisCross = true;

    this->enableOverlayPainting( true );
    this->setReleaseOGLResourcesEachFrame( true );

    // Info Text
    m_infoLabel = new QLabel();
    m_infoLabel->setObjectName( "InfoLabel" );
    m_infoLabel->setFrameShape( QFrame::Box );
    m_infoLabel->setFrameShadow( QFrame::Plain );
    m_infoLabel->setMinimumWidth( 275 );
    m_infoLabel->setFont( font );
    m_showInfoText = true;

    // Version info label
    m_versionInfoLabel = new QLabel();
    m_versionInfoLabel->setFrameShape( QFrame::NoFrame );
    m_versionInfoLabel->setAlignment( Qt::AlignRight );
    m_versionInfoLabel->setText(
        QString( "%1 v%2" ).arg( RI_APPLICATION_NAME, RiaApplication::getVersionStringApp( false ) ) );
    m_versionInfoLabel->setFont( font );
    m_showVersionInfo = true;

    // Z scale label
    m_zScaleLabel = new QLabel();
    m_zScaleLabel->setFrameShape( QFrame::NoFrame );
    m_zScaleLabel->setAlignment( Qt::AlignLeft );
    m_zScaleLabel->setText( QString( "Z: " ) );
    m_zScaleLabel->setFont( font );
    m_showZScaleLabel    = true;
    m_hideZScaleCheckbox = false;

    // Animation progress bar
    m_animationProgress = new caf::QStyledProgressBar( "AnimationProgress" );
    m_animationProgress->setFormat( "Time Step: %v/%m" );
    m_animationProgress->setTextVisible( true );
    m_animationProgress->setAlignment( Qt::AlignCenter );
    m_animationProgress->setObjectName( "AnimationProgress" );
    m_animationProgress->setFont( font );

    m_showAnimProgress = false;

    // Histogram
    m_histogramWidget = new RiuSimpleHistogramWidget( "HistogramWidget" );
    m_showHistogram   = false;

    m_viewerCommands = new RiuViewerCommands( this );

    if ( RiaRegressionTestRunner::instance()->isRunningRegressionTests() )
    {
        QFont regTestFont = m_infoLabel->font();
        regTestFont.setPixelSize( 11 );

        m_infoLabel->setFont( regTestFont );
        m_versionInfoLabel->setFont( regTestFont );
        m_animationProgress->setFont( regTestFont );
        m_histogramWidget->setFont( regTestFont );
        m_zScaleLabel->setFont( regTestFont );
    }

    // When a context menu is created in the viewer is, and the action triggered is displaying a dialog,
    // the context menu of QMainWindow is displayed after the action has finished
    // Setting this policy will make sure the handling is not deferred to the widget's parent,
    // which solves the problem
    setContextMenuPolicy( Qt::PreventContextMenu );

    m_gridBoxGenerator           = new RivGridBoxGenerator;
    m_comparisonGridBoxGenerator = new RivGridBoxGenerator;

    m_cursorPositionDomainCoords = cvf::Vec3d::UNDEFINED;
    m_windowEdgeAxisOverlay      = new RivWindowEdgeAxesOverlayItem( standardFont );
    m_showWindowEdgeAxes         = false;

    m_selectionVisualizerManager = new caf::PdmUiSelection3dEditorVisualizer( this );

    m_scaleLegend = new caf::OverlayScaleLegend( standardFont );
    m_scaleLegend->setOrientation( caf::OverlayScaleLegend::HORIZONTAL );

    m_comparisonWindowMover = new RiuComparisonViewMover( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuViewer::~RiuViewer()
{
    if ( m_rimView )
    {
        m_rimView->setCameraPosition( m_mainCamera->viewMatrix() );
        m_rimView->setCameraPointOfInterest( pointOfInterest() );
    }

    delete m_infoLabel;
    delete m_animationProgress;
    delete m_histogramWidget;
    delete m_gridBoxGenerator;
    delete m_comparisonGridBoxGenerator;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::clearRimView()
{
    m_rimView = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::setDefaultView()
{
    cvf::BoundingBox bb = m_mainRendering->boundingBox();
    if ( !bb.isValid() )
    {
        bb.add( cvf::Vec3d( -1, -1, -1 ) );
        bb.add( cvf::Vec3d( 1, 1, 1 ) );
    }

    if ( m_mainCamera->projection() == cvf::Camera::PERSPECTIVE )
    {
        m_mainCamera->setProjectionAsPerspective( 40.0, RI_MIN_NEARPLANE_DISTANCE, 1000 );
    }
    else
    {
        if ( bb.isValid() )
        {
            m_mainCamera->setProjectionAsOrtho( bb.extent().length(), RI_MIN_NEARPLANE_DISTANCE, 1000 );
        }
    }

    m_mainCamera->fitView( bb, -cvf::Vec3d::Z_AXIS, cvf::Vec3d::Y_AXIS );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::mouseReleaseEvent( QMouseEvent* event )
{
    if ( !this->canRender() ) return;

    if ( event->button() == Qt::LeftButton )
    {
        QPoint diffPoint = event->pos() - m_lastMousePressPosition;
        if ( diffPoint.manhattanLength() > 3 )
        {
            // We are possibly in navigation mode, only clean press event will launch
            return;
        }

        if ( !m_infoLabelOverlayArea.isNull() )
        {
            if ( m_infoLabelOverlayArea.contains( event->x(), event->y() ) )
            {
                m_rimView->selectOverlayInfoConfig();

                return;
            }
        }

        m_viewerCommands->handlePickAction( event->x(), event->y(), event->modifiers() );

        return;
    }
    else if ( event->button() == Qt::RightButton )
    {
        QPoint diffPoint = event->pos() - m_lastMousePressPosition;
        if ( diffPoint.manhattanLength() > 3 )
        {
            // We are possibly in navigation mode, only clean press event will launch
            return;
        }

        m_viewerCommands->displayContextMenu( event );
        return;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::slotEndAnimation()
{
    CVF_ASSERT( m_mainRendering.notNull() );

    if ( m_rimView ) m_rimView->endAnimation();

    caf::Viewer::slotEndAnimation();

    caf::EffectGenerator::releaseUnreferencedEffects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::slotSetCurrentFrame( int frameIndex )
{
    setCurrentFrame( frameIndex );

    if ( m_rimView )
    {
        RimViewLinker* viewLinker = m_rimView->assosiatedViewLinker();
        if ( viewLinker )
        {
            viewLinker->updateTimeStep( dynamic_cast<RimGridView*>( m_rimView.p() ), frameIndex );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RiuViewer::pointOfInterest()
{
    return m_navigationPolicy->pointOfInterest();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::setPointOfInterest( cvf::Vec3d poi )
{
    m_navigationPolicy->setPointOfInterest( poi );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::setOwnerReservoirView( RiuViewerToViewInterface* owner )
{
    m_rimView = owner;

    m_viewerCommands->setOwnerView( dynamic_cast<Rim3dView*>( owner ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::paintOverlayItems( QPainter* painter )
{
    // Update the legend layout on every redraw as the legends stores their own position, 
    // and when they are shared between views the positions are overwritten.
    updateLegendLayout();

    int columnWidth = 200;

    int edgeAxisFrameBorderWidth  = m_showWindowEdgeAxes ? m_windowEdgeAxisOverlay->frameBorderWidth() : 0;
    int edgeAxisFrameBorderHeight = m_showWindowEdgeAxes ? m_windowEdgeAxisOverlay->frameBorderHeight() : 0;

    int margin = 5;
    int yPos   = margin + edgeAxisFrameBorderHeight;

    bool showAnimBar = false;
    if ( isAnimationActive() && frameCount() > 1 ) showAnimBar = true;

    if ( m_showInfoText ) columnWidth = CVF_MAX( columnWidth, m_infoLabel->sizeHint().width() );

    int columnPos = this->width() - columnWidth - margin - edgeAxisFrameBorderWidth;

    if ( showAnimBar && m_showAnimProgress )
    {
        QString stepName = m_rimView->timeStepName( currentFrameIndex() );
        m_animationProgress->setFormat( "Time Step: %v/%m " + stepName );
        m_animationProgress->setMinimum( 0 );
        m_animationProgress->setMaximum( static_cast<int>( frameCount() ) - 1 );
        m_animationProgress->setValue( currentFrameIndex() );
        m_animationProgress->resize( columnWidth, m_animationProgress->sizeHint().height() );

        m_animationProgress->render( painter, QPoint( columnPos, yPos ) );
        yPos += m_animationProgress->height() + margin;
    }

    if ( m_showInfoText )
    {
        QPoint topLeft = QPoint( columnPos, yPos );
        m_infoLabel->resize( columnWidth, m_infoLabel->sizeHint().height() );
        m_infoLabel->render( painter, topLeft );

        m_infoLabelOverlayArea.setTopLeft( topLeft );
        m_infoLabelOverlayArea.setBottom( yPos + m_infoLabel->height() );
        m_infoLabelOverlayArea.setRight( columnPos + columnWidth );

        yPos += m_infoLabel->height() + margin;
    }
    else
    {
        m_infoLabelOverlayArea = QRect();
    }

    if ( m_showHistogram )
    {
        m_histogramWidget->resize( columnWidth, 40 );
        m_histogramWidget->render( painter, QPoint( columnPos, yPos ) );
        // yPos +=  m_histogramWidget->height() + margin;
    }

    if ( m_showVersionInfo ) // Version Label
    {
        QSize  size( m_versionInfoLabel->sizeHint().width(), m_versionInfoLabel->sizeHint().height() );
        QPoint pos( this->width() - size.width() - margin - edgeAxisFrameBorderWidth,
                    this->height() - size.height() - margin - edgeAxisFrameBorderHeight );
        m_versionInfoLabel->resize( size.width(), size.height() );
        m_versionInfoLabel->render( painter, pos );
    }

    if ( m_showZScaleLabel ) // Z scale Label
    {
        QSize  size( m_zScaleLabel->sizeHint().width(), m_zScaleLabel->sizeHint().height() );
        QPoint pos( margin + edgeAxisFrameBorderWidth, margin + edgeAxisFrameBorderHeight );
        m_zScaleLabel->resize( size.width(), size.height() );
        m_zScaleLabel->render( painter, pos );
    }

    if ( !m_cursorPositionDomainCoords.isUndefined() )
    {
        if ( mainCamera() )
        {
            cvf::ref<caf::DisplayCoordTransform> trans = m_rimView->displayCoordTransform();

            cvf::Vec3d displayCoord = trans->transformToDisplayCoord( m_cursorPositionDomainCoords );

            cvf::Vec3d screenCoords;
            if ( mainCamera()->project( displayCoord, &screenCoords ) )
            {
                int    translatedMousePosY = height() - screenCoords.y();
                QPoint centerPos( screenCoords.x(), translatedMousePosY );

                // Draw a cross hair marker
                int markerHalfLength = 6;

                painter->drawLine( centerPos.x(),
                                   centerPos.y() - markerHalfLength,
                                   centerPos.x(),
                                   centerPos.y() + markerHalfLength );
                painter->drawLine( centerPos.x() - markerHalfLength,
                                   centerPos.y(),
                                   centerPos.x() + markerHalfLength,
                                   centerPos.y() );
            }
        }
    }

    m_comparisonWindowMover->paintMoverHandles( painter );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::setInfoText( QString text )
{
    m_infoLabel->setText( text );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::hideZScaleCheckbox( bool hide )
{
    m_hideZScaleCheckbox = hide;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::showZScaleLabel( bool enable )
{
    m_showZScaleLabel = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::setZScale( int scale )
{
    m_zScaleLabel->setText( QString( "Z: %1" ).arg( scale ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::showInfoText( bool enable )
{
    m_showInfoText = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::showVersionInfo( bool enable )
{
    m_showVersionInfo = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::setHistogram( double min, double max, const std::vector<size_t>& histogram )
{
    m_histogramWidget->setHistogramData( min, max, histogram );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::setHistogramPercentiles( double pmin, double pmax, double mean )
{
    m_histogramWidget->setPercentiles( pmin, pmax );
    m_histogramWidget->setMean( mean );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::showGridBox( bool enable )
{
    this->removeStaticModel( m_gridBoxGenerator->model() );
    this->removeStaticModel( m_comparisonGridBoxGenerator->model() );

    if ( enable )
    {
        this->addStaticModelOnce( m_gridBoxGenerator->model(), false );
        this->addStaticModelOnce( m_comparisonGridBoxGenerator->model(), true );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::showAnimationProgress( bool enable )
{
    m_showAnimProgress = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::showHistogram( bool enable )
{
    m_showHistogram = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::mousePressEvent( QMouseEvent* event )
{
    m_lastMousePressPosition = event->pos();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::removeAllColorLegends()
{
    for ( size_t i = 0; i < m_visibleLegends.size(); i++ )
    {
        overlayItemsRendering()->removeOverlayItem( m_visibleLegends[i].p() );
    }

    m_visibleLegends.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::addColorLegendToBottomLeftCorner( caf::TitledOverlayFrame* addedLegend )
{
    RiaGuiApplication* app = RiaGuiApplication::instance();
    CVF_ASSERT( app );
    RiaPreferences* preferences      = app->preferences();
    cvf::Rendering* overlayRendering = overlayItemsRendering();
    CVF_ASSERT( preferences );
    CVF_ASSERT( overlayRendering );

    if ( addedLegend )
    {
        cvf::Color4f backgroundColor = mainCamera()->viewport()->clearColor();
        backgroundColor.a()          = 0.8f;
        cvf::Color3f frameColor( backgroundColor.r(), backgroundColor.g(), backgroundColor.b() );
        updateLegendTextAndTickMarkColor( addedLegend );

        overlayRendering->addOverlayItem( addedLegend );
        addedLegend->enableBackground( preferences->showLegendBackground() );
        addedLegend->setBackgroundColor( backgroundColor );
        addedLegend->setBackgroundFrameColor(
            cvf::Color4f( RiaColorTools::computeOffsetColor( frameColor, 0.3f ), 0.9f ) );
        addedLegend->setFont( app->defaultSceneFont() );

        if ( !m_visibleLegends.contains( addedLegend ) )
        {
            m_visibleLegends.push_back( addedLegend );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::removeColorLegend( caf::TitledOverlayFrame* legend )
{
    overlayItemsRendering()->removeOverlayItem( legend );
    m_visibleLegends.erase( legend );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::updateLegendLayout()
{
    int viewPortHeight = static_cast<int>( m_mainCamera->viewport()->height() );

    const float maxFreeLegendHeight  = 0.7f * viewPortHeight;
    const int   border               = 3;
    int         edgeAxisBorderWidth  = m_showWindowEdgeAxes ? m_windowEdgeAxisOverlay->frameBorderWidth() : 0;
    int         edgeAxisBorderHeight = m_showWindowEdgeAxes ? m_windowEdgeAxisOverlay->frameBorderHeight() : 0;

    {
        int xPos = border + edgeAxisBorderWidth;
        int yPos = border + edgeAxisBorderHeight;

        std::vector<caf::TitledOverlayFrame*> standardHeightLegends;

        // Place the legends needing the full height, and sort out the standard height legends

        for ( cvf::ref<caf::TitledOverlayFrame> legend : m_visibleLegends )
        {
            cvf::Vec2ui prefSize = legend->preferredSize();
            if ( prefSize.y() > maxFreeLegendHeight )
            {
                int legendWidth = prefSize.x();
                legend->setLayoutFixedPosition( cvf::Vec2i( xPos, yPos ) );
                legend->setRenderSize(
                    cvf::Vec2ui( legendWidth, viewPortHeight - 2 * border - 2 * edgeAxisBorderHeight ) );
                xPos += legendWidth + border;
            }
            else
            {
                standardHeightLegends.push_back( legend.p() );
            }
        }

        // Place the rest of the legends in columns that fits within the screen height

        int                                   maxColumnWidht = 0;
        std::vector<caf::TitledOverlayFrame*> columnLegends;

        for ( caf::TitledOverlayFrame* legend : standardHeightLegends )
        {
            cvf::Vec2ui prefSize = legend->preferredSize();

            // Check if we need a new column
            if ( ( yPos + (int)prefSize.y() + border ) > viewPortHeight )
            {
                xPos += border + maxColumnWidht;
                yPos = border + edgeAxisBorderHeight;

                // Set same width to all legends in the column
                for ( caf::TitledOverlayFrame* columnLegend : columnLegends )
                {
                    columnLegend->setRenderSize( cvf::Vec2ui( maxColumnWidht, columnLegend->renderSize().y() ) );
                }
                maxColumnWidht = 0;
                columnLegends.clear();
            }

            legend->setLayoutFixedPosition( cvf::Vec2i( xPos, yPos ) );
            legend->setRenderSize( cvf::Vec2ui( prefSize.x(), prefSize.y() ) );
            columnLegends.push_back( legend );

            yPos += legend->renderSize().y() + border;
            maxColumnWidht = std::max( maxColumnWidht, (int)prefSize.x() );
        }

        // Set same width to all legends in the last column

        for ( caf::TitledOverlayFrame* legend : columnLegends )
        {
            legend->setRenderSize( cvf::Vec2ui( maxColumnWidht, legend->renderSize().y() ) );
        }
    }

    {
        int  margin           = 5;
        auto scaleLegendSize  = m_scaleLegend->renderSize();
        auto otherItemsHeight = m_versionInfoLabel->sizeHint().height();

        const int xPos = width() - (int)scaleLegendSize.x() - margin - edgeAxisBorderWidth;
        const int yPos = margin + edgeAxisBorderHeight + margin + otherItemsHeight;

        m_scaleLegend->setLayoutFixedPosition( {xPos, yPos} );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::enableNavigationRotation( bool enable )
{
    auto tbNavPol                 = dynamic_cast<caf::TrackBallBasedNavigation*>( m_navigationPolicy.p() );
    m_isNavigationRotationEnabled = enable;

    if ( tbNavPol ) tbNavPol->enableRotation( m_isNavigationRotationEnabled );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::updateNavigationPolicy()
{
    switch ( RiaGuiApplication::instance()->navigationPolicy() )
    {
        case RiaGuiApplication::NAVIGATION_POLICY_CAD:
            setNavigationPolicy( new RiuCadNavigation );
            break;

        case RiaGuiApplication::NAVIGATION_POLICY_CEETRON:
            setNavigationPolicy( new caf::CeetronPlusNavigation );
            break;

        case RiaGuiApplication::NAVIGATION_POLICY_GEOQUEST:
            setNavigationPolicy( new RiuGeoQuestNavigation );
            break;

        case RiaGuiApplication::NAVIGATION_POLICY_RMS:
            setNavigationPolicy( new RiuRmsNavigation );
            break;

        default:
            CVF_ASSERT( 0 );
            break;
    }

    enableNavigationRotation( m_isNavigationRotationEnabled );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::navigationPolicyUpdate()
{
    caf::Viewer::navigationPolicyUpdate();
    ownerViewWindow()->viewNavigationChanged();
    if ( m_rimView )
    {
        RimViewLinker* viewLinker = m_rimView->assosiatedViewLinker();
        if ( viewLinker )
        {
            viewLinker->updateCamera( dynamic_cast<RimGridView*>( m_rimView.p() ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::setCurrentFrame( int frameIndex )
{
    CVF_ASSERT( m_mainRendering.notNull() );

    if ( m_rimView ) m_rimView->setCurrentTimeStepAndUpdate( frameIndex );

    animationControl()->setCurrentFrameOnly( frameIndex );

    caf::Viewer::slotSetCurrentFrame( frameIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::showAxisCross( bool enable )
{
    overlayItemsRendering()->removeOverlayItem( m_axisCross.p() );

    if ( enable )
    {
        overlayItemsRendering()->addOverlayItem( m_axisCross.p() );
    }
    m_showAxisCross = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuViewerToViewInterface* RiuViewer::ownerReservoirView()
{
    return m_rimView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewWindow* RiuViewer::ownerViewWindow() const
{
    return dynamic_cast<RimViewWindow*>( m_rimView.p() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::optimizeClippingPlanes()
{
    if ( m_showWindowEdgeAxes )
    {
        m_windowEdgeAxisOverlay->setDisplayCoordTransform( m_rimView->displayCoordTransform().p() );
        m_windowEdgeAxisOverlay->updateFromCamera( this->mainCamera() );
    }

    m_gridBoxGenerator->updateFromCamera( mainCamera() );
    m_comparisonGridBoxGenerator->updateFromCamera( comparisonMainCamera() );

    m_scaleLegend->setDisplayCoordTransform( m_rimView->displayCoordTransform().p() );
    m_scaleLegend->updateFromCamera( mainCamera() );

    caf::Viewer::optimizeClippingPlanes();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::resizeGL( int width, int height )
{
    caf::Viewer::resizeGL( width, height );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::mouseMoveEvent( QMouseEvent* mouseEvent )
{
    if ( m_rimView )
    {
        RimViewLinker* viewLinker = m_rimView->assosiatedViewLinker();
        if ( viewLinker )
        {
            int translatedMousePosX = mouseEvent->pos().x();
            int translatedMousePosY = height() - mouseEvent->pos().y();

            cvf::Vec3d displayCoord( 0, 0, 0 );
            if ( mainCamera()->unproject( cvf::Vec3d( static_cast<double>( translatedMousePosX ),
                                                      static_cast<double>( translatedMousePosY ),
                                                      0 ),
                                          &displayCoord ) )
            {
                if ( m_cursorPositionDomainCoords != cvf::Vec3d::UNDEFINED )
                {
                    // Reset the extra cursor if the view currently is receiving mouse cursor events
                    // Set undefined and redraw to remove the previously displayed cursor
                    m_cursorPositionDomainCoords = cvf::Vec3d::UNDEFINED;

                    update();
                }

                cvf::ref<caf::DisplayCoordTransform> trans       = m_rimView->displayCoordTransform();
                cvf::Vec3d                           domainCoord = trans->transformToDomainCoord( displayCoord );

                viewLinker->updateCursorPosition( dynamic_cast<RimGridView*>( m_rimView.p() ), domainCoord );
            }
        }
    }

    caf::Viewer::mouseMoveEvent( mouseEvent );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::enterEvent( QEvent* e )
{
    if ( s_hoverCursor )
    {
        QApplication::setOverrideCursor( *s_hoverCursor );
    }
    caf::Viewer::enterEvent( e );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::leaveEvent( QEvent* )
{
    QApplication::restoreOverrideCursor();

    if ( m_rimView && m_rimView->assosiatedViewLinker() )
    {
        RimViewLinker* viewLinker = m_rimView->assosiatedViewLinker();
        viewLinker->updateCursorPosition( dynamic_cast<RimGridView*>( m_rimView.p() ), cvf::Vec3d::UNDEFINED );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::updateGridBoxData( double                  scaleZ,
                                   const cvf::Vec3d&       displayModelOffset,
                                   const cvf::Color3f&     backgroundColor,
                                   const cvf::BoundingBox& domainCoordBoundingBox )
{
    m_gridBoxGenerator->setScaleZ( scaleZ );
    m_gridBoxGenerator->setDisplayModelOffset( displayModelOffset );
    m_gridBoxGenerator->updateFromBackgroundColor( backgroundColor );
    m_gridBoxGenerator->setGridBoxDomainCoordBoundingBox( domainCoordBoundingBox );

    m_gridBoxGenerator->createGridBoxParts();

    m_comparisonGridBoxGenerator->setScaleZ( scaleZ );
    cvf::Vec3d unscaledComparisonOffset = comparisonViewEyePointOffset();

    unscaledComparisonOffset.z() /= scaleZ;

    m_comparisonGridBoxGenerator->setDisplayModelOffset( displayModelOffset - unscaledComparisonOffset );
    m_comparisonGridBoxGenerator->updateFromBackgroundColor( backgroundColor );
    m_comparisonGridBoxGenerator->setGridBoxDomainCoordBoundingBox( domainCoordBoundingBox );

    m_comparisonGridBoxGenerator->createGridBoxParts();

    m_selectionVisualizerManager->updateVisibleEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::showEdgeTickMarksXY( bool enable, bool showAxisLines )
{
    overlayItemsRendering()->removeOverlayItem( m_windowEdgeAxisOverlay.p() );

    if ( enable )
    {
        m_windowEdgeAxisOverlay->setDomainAxes( RivWindowEdgeAxesOverlayItem::XY_AXES );
        m_windowEdgeAxisOverlay->setIsSwitchingYAxisSign( false );
        m_windowEdgeAxisOverlay->setShowAxisLines( showAxisLines );
        overlayItemsRendering()->addOverlayItem( m_windowEdgeAxisOverlay.p() );
    }

    m_showWindowEdgeAxes = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::showEdgeTickMarksXZ( bool enable, bool showAxisLines )
{
    overlayItemsRendering()->removeOverlayItem( m_windowEdgeAxisOverlay.p() );

    if ( enable )
    {
        m_windowEdgeAxisOverlay->setDomainAxes( RivWindowEdgeAxesOverlayItem::XZ_AXES );
        m_windowEdgeAxisOverlay->setIsSwitchingYAxisSign( true );
        m_windowEdgeAxisOverlay->setShowAxisLines( showAxisLines );
        overlayItemsRendering()->addOverlayItem( m_windowEdgeAxisOverlay.p() );
    }

    m_showWindowEdgeAxes = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::updateAnnotationItems()
{
    updateTextAndTickMarkColorForOverlayItems();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::setAxisLabels( const cvf::String& xLabel, const cvf::String& yLabel, const cvf::String& zLabel )
{
    m_axisCross->setAxisLabels( xLabel, yLabel, zLabel );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RiuViewer::lastPickPositionInDomainCoords() const
{
    CVF_ASSERT( m_viewerCommands );

    return m_viewerCommands->lastPickPositionInDomainCoords();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::OverlayItem* RiuViewer::pickFixedPositionedLegend( int winPosX, int winPosY )
{
    int translatedMousePosX = winPosX;
    int translatedMousePosY = height() - winPosY;

    for ( auto overlayItem : m_visibleLegends )
    {
        if ( overlayItem->layoutScheme() == cvf::OverlayItem::FIXED_POSITION &&
             overlayItem->pick( translatedMousePosX,
                                translatedMousePosY,
                                overlayItem->fixedPosition(),
                                overlayItem->renderSize() ) )
        {
            return overlayItem.p();
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::setCursorPosition( const cvf::Vec3d& domainCoord )
{
    if ( m_cursorPositionDomainCoords != domainCoord )
    {
        m_cursorPositionDomainCoords = domainCoord;

        update();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::ref<cvf::Part>> RiuViewer::visibleParts()
{
    std::vector<cvf::ref<cvf::Part>> partsMatchingEnableMask;

    if ( m_mainRendering.notNull() )
    {
        auto        enableMask = m_mainRendering->enableMask();
        cvf::Scene* scene      = currentScene();

        for ( cvf::uint i = 0; i < scene->modelCount(); i++ )
        {
            cvf::Model* model = scene->model( i );
            if ( enableMask & model->partEnableMask() )
            {
                cvf::Collection<cvf::Part> partCollection;
                model->allParts( &partCollection );

                for ( const auto& p : partCollection )
                {
                    if ( enableMask & p->enableMask() )
                    {
                        partsMatchingEnableMask.push_back( p );
                    }
                }
            }
        }
    }

    return partsMatchingEnableMask;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::showScaleLegend( bool show )
{
    if ( show )
    {
        if ( m_scaleLegend->orientation() == caf::OverlayScaleLegend::HORIZONTAL )
            m_scaleLegend->setRenderSize( {280, 45} );
        else
            m_scaleLegend->setRenderSize( {50, 280} );

        overlayItemsRendering()->addOverlayItem( m_scaleLegend.p() );
    }
    else
    {
        overlayItemsRendering()->removeOverlayItem( m_scaleLegend.p() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::setHoverCursor( const QCursor& cursor )
{
    s_hoverCursor.reset( new QCursor( cursor ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::clearHoverCursor()
{
    s_hoverCursor.reset();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::updateFonts()
{
    cvf::Font* standardFont = RiaGuiApplication::instance()->defaultSceneFont();
    overlayItemsRendering()->removeOverlayItem( m_axisCross.p() );

    m_axisCross = new cvf::OverlayAxisCross( m_mainCamera.p(), standardFont );
    m_axisCross->setAxisLabels( "X", "Y", "Z" );
    m_axisCross->setLayout( cvf::OverlayItem::VERTICAL, cvf::OverlayItem::BOTTOM_RIGHT );
    overlayItemsRendering()->addOverlayItem( m_axisCross.p() );
    m_showAxisCross = true;

    QFont font = QApplication::font();
    font.setPointSize(
        RiaFontCache::pointSizeFromFontSizeEnum( RiaApplication::instance()->preferences()->defaultSceneFontSize() ) );

    m_zScaleLabel->setFont( font );
    m_infoLabel->setFont( font );
    m_animationProgress->setFont( font );
    m_versionInfoLabel->setFont( font );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::updateLegendTextAndTickMarkColor( cvf::OverlayItem* legend )
{
    if ( m_rimView.isNull() ) return;

    cvf::Color3f contrastColor = computeContrastColor();

    caf::OverlayScalarMapperLegend* scalarMapperLegend = dynamic_cast<caf::OverlayScalarMapperLegend*>( legend );
    if ( scalarMapperLegend )
    {
        scalarMapperLegend->setTextColor( contrastColor );
        scalarMapperLegend->setLineColor( contrastColor );
    }

    caf::CategoryLegend* categoryLegend = dynamic_cast<caf::CategoryLegend*>( legend );
    if ( categoryLegend )
    {
        categoryLegend->setTextColor( contrastColor );
        categoryLegend->setLineColor( contrastColor );
    }

    RivTernarySaturationOverlayItem* ternaryItem = dynamic_cast<RivTernarySaturationOverlayItem*>( legend );
    if ( ternaryItem )
    {
        ternaryItem->setAxisLabelsColor( contrastColor );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::updateTextAndTickMarkColorForOverlayItems()
{
    for ( size_t i = 0; i < m_visibleLegends.size(); i++ )
    {
        updateLegendTextAndTickMarkColor( m_visibleLegends.at( i ) );
    }

    updateAxisCrossTextColor();

    updateOverlayItemsStyle();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::updateAxisCrossTextColor()
{
    cvf::Color3f contrastColor = computeContrastColor();

    m_axisCross->setAxisLabelsColor( contrastColor );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::updateOverlayItemsStyle()
{
    QColor backgroundColor;
    QColor backgroundFrameColor;
    QColor contrastColor;
    {
        cvf::Color4f cvf_backgroundColor = mainCamera()->viewport()->clearColor();
        cvf_backgroundColor.a()          = 0.65f;

        cvf::Color4f cvf_backgroundFrameColor =
            cvf::Color4f( RiaColorTools::computeOffsetColor( cvf_backgroundColor.toColor3f(), 0.3f ), 0.9f );

        cvf::Color3f cvf_contrastColor = computeContrastColor();

        backgroundColor      = RiaColorTools::toQColor( cvf_backgroundColor );
        backgroundFrameColor = RiaColorTools::toQColor( cvf_backgroundFrameColor );
        contrastColor        = RiaColorTools::toQColor( cvf_contrastColor );
    }

    QPalette p = QApplication::palette();

    p.setColor( QPalette::Window, backgroundColor );
    p.setColor( QPalette::Base, backgroundColor );

    p.setColor( QPalette::WindowText, contrastColor );

    p.setColor( QPalette::Shadow, backgroundFrameColor );
    p.setColor( QPalette::Light, backgroundFrameColor );
    p.setColor( QPalette::Midlight, backgroundFrameColor );
    p.setColor( QPalette::Dark, backgroundFrameColor );
    p.setColor( QPalette::Mid, backgroundFrameColor );

    m_infoLabel->setStyleSheet( caf::StyleSheetTools::createFrameStyleSheet( "QLabel",
                                                                             "InfoLabel",
                                                                             contrastColor,
                                                                             backgroundColor,
                                                                             backgroundFrameColor ) );
    m_histogramWidget->setStyleSheet( caf::StyleSheetTools::createFrameStyleSheet( "",
                                                                                   "HistogramWidget",
                                                                                   contrastColor,
                                                                                   backgroundColor,
                                                                                   backgroundFrameColor ) );
    m_histogramWidget->setPalette( p );

    m_versionInfoLabel->setPalette( p );
    m_zScaleLabel->setPalette( p );

    QColor progressColor( Qt::green );
    progressColor.setAlphaF( 0.8f );
    backgroundColor.setAlphaF( 0.8f );
    m_animationProgress->setTextBackgroundAndProgressColor( contrastColor,
                                                            backgroundColor,
                                                            backgroundFrameColor,
                                                            progressColor );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RiuViewer::computeContrastColor() const
{
    cvf::Color3f contrastColor = RiaColorTools::brightContrastColor();

    if ( m_rimView.notNull() )
    {
        contrastColor = RiaColorTools::contrastColor( m_rimView->backgroundColor() );
    }

    return contrastColor;
}
