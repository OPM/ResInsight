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
#include "RiaGuiApplication.h"
#include "RiaPreferences.h"
#include "RiaRegressionTestRunner.h"

#include "Rim3dView.h"
#include "RimCase.h"
#include "RimViewLinker.h"

#include "RivGridBoxGenerator.h"
#include "RivTernarySaturationOverlayItem.h"
#include "WindowEdgeAxesOverlayItem/RivWindowEdgeAxesOverlayItem.h"

#include "RiuCadNavigation.h"
#include "RiuComparisonViewMover.h"
#include "RiuGeoQuestNavigation.h"
#include "RiuGuiTheme.h"
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
#include "cvfOverlayAxisCross.h"
#include "cvfOverlayItem.h"
#include "cvfRendering.h"
#include "cvfScene.h"

#include <algorithm>

#include <QLabel>

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
RiuViewer::RiuViewer( QWidget* parent )
    : caf::Viewer( parent )
    , m_isNavigationRotationEnabled( true )
    , m_zScale( 1.0 )
{
    cvf::Font* standardFont = RiaGuiApplication::instance()->defaultSceneFont();
    QFont      font         = QApplication::font();

    auto viewFontSize = RiaPreferences::current()->defaultSceneFontSize();
    font.setPointSize( caf::FontTools::absolutePointSize( viewFontSize ) );

    m_axisCross = new cvf::OverlayAxisCross( m_mainCamera.p(), standardFont );
    m_axisCross->setAxisLabels( "X", "Y", "Z" );
    m_axisCross->setLayout( cvf::OverlayItem::VERTICAL, cvf::OverlayItem::BOTTOM_RIGHT );
    overlayItemsRendering()->addOverlayItem( m_axisCross.p() );
    m_showAxisCross = true;

    enableOverlayPainting( true );
    setReleaseOGLResourcesEachFrame( true );

    // Info Text
    m_infoLabel = new QLabel();
    m_infoLabel->setObjectName( "InfoLabel" );
    m_infoLabel->setFrameShape( QFrame::Box );
    m_infoLabel->setFrameShadow( QFrame::Plain );
    m_infoLabel->setMinimumWidth( 275 );
    m_infoLabel->setFont( font );
    m_showInfoText = true;

    m_shortInfoLabel = new QLabel();
    m_shortInfoLabel->setObjectName( "ShortInfoLabel" );
    m_shortInfoLabel->setFrameShape( QFrame::Box );
    m_shortInfoLabel->setFrameShadow( QFrame::Plain );
    m_shortInfoLabel->setMinimumWidth( 100 );
    m_shortInfoLabel->setFont( font );

    m_shortInfoLabelCompView = new QLabel();
    m_shortInfoLabelCompView->setObjectName( "ShortInfoLabelCompView" );
    m_shortInfoLabelCompView->setFrameShape( QFrame::Box );
    m_shortInfoLabelCompView->setFrameShadow( QFrame::Plain );
    m_shortInfoLabelCompView->setMinimumWidth( 100 );
    m_shortInfoLabelCompView->setFont( font );

    // Version info label
    m_versionInfoLabel = new QLabel();
    m_versionInfoLabel->setFrameShape( QFrame::NoFrame );
    m_versionInfoLabel->setAlignment( Qt::AlignRight );
    m_versionInfoLabel->setObjectName( "VersionInfo" );
    m_versionInfoLabel->setText( QString( "%1 v%2" ).arg( RI_APPLICATION_NAME, RiaApplication::getVersionStringApp( false ) ) );
    m_versionInfoLabel->setFont( font );
    m_showVersionInfo = true;

    // Z scale label
    m_zScaleLabel = new QLabel();
    m_zScaleLabel->setFrameShape( QFrame::NoFrame );
    m_zScaleLabel->setAlignment( Qt::AlignLeft );
    m_zScaleLabel->setObjectName( "ZScaleLabel" );
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

    m_animationProgressCompView = new caf::QStyledProgressBar( "AnimationProgress" );
    m_animationProgressCompView->setFormat( "Time Step: %v/%m" );
    m_animationProgressCompView->setTextVisible( true );
    m_animationProgressCompView->setAlignment( Qt::AlignCenter );
    m_animationProgressCompView->setObjectName( "AnimationProgress" );
    m_animationProgressCompView->setFont( font );

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
        m_shortInfoLabel->setFont( regTestFont );
        m_shortInfoLabelCompView->setFont( regTestFont );
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

    auto backgroundColor = RiuGuiTheme::getColorByVariableName( "backgroundColor1" );
    auto textColor       = RiuGuiTheme::getColorByVariableName( "textColor" );
    m_windowEdgeAxisOverlay->setFrameColor( cvf::Color4f( RiaColorTools::fromQColorTo3f( backgroundColor ) ) );
    m_windowEdgeAxisOverlay->setTextColor( RiaColorTools::fromQColorTo3f( textColor ) );

    m_selectionVisualizerManager = new caf::PdmUiSelection3dEditorVisualizer( this );

    m_scaleLegend = new caf::OverlayScaleLegend( standardFont );
    m_scaleLegend->setOrientation( caf::OverlayScaleLegend::HORIZONTAL );

    m_comparisonWindowMover = new RiuComparisonViewMover( this );
    setComparisonViewToFollowAnimation( false );

    m_fontPointSize = caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultSceneFontSize() );
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
    delete m_shortInfoLabel;
    delete m_shortInfoLabelCompView;
    delete m_animationProgress;
    delete m_animationProgressCompView;
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
void RiuViewer::setDefaultView( const cvf::Vec3d& dir, const cvf::Vec3d& up )
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

    m_mainCamera->fitView( bb, dir, up );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::mouseReleaseEvent( QMouseEvent* event )
{
    if ( !canRender() ) return;

    if ( event->button() == Qt::LeftButton )
    {
        QPoint diffPoint = event->pos() - m_lastMousePressPosition;
        if ( diffPoint.manhattanLength() > 3 )
        {
            // We are possibly in navigation mode, only clean press event will launch
            return;
        }

        if ( !m_infoPickArea.isNull() )
        {
            if ( m_infoPickArea.contains( event->x(), event->y() ) )
            {
                m_rimView->selectOverlayInfoConfig();

                return;
            }
        }

        if ( !m_infoPickAreaCompView.isNull() )
        {
            if ( m_infoPickAreaCompView.contains( event->x(), event->y() ) )
            {
                Rim3dView* compView = dynamic_cast<Rim3dView*>( m_rimView.p() )->activeComparisonView();

                if ( compView ) compView->selectOverlayInfoConfig();

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

    auto* view = dynamic_cast<Rim3dView*>( m_rimView.p() );
    if ( view )
    {
        RimViewLinker* viewLinker = view->assosiatedViewLinker();
        if ( viewLinker )
        {
            viewLinker->updateTimeStep( view, frameIndex );
        }

        // Update  views using this as comparison
        std::set<Rim3dView*> containingViews = view->viewsUsingThisAsComparisonView();

        for ( auto contView : containingViews )
        {
            contView->updateDisplayModelForCurrentTimeStepAndRedraw();
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

    int margin    = 5;
    int startYPos = margin + edgeAxisFrameBorderHeight;
    int yPos      = startYPos;

    bool showAnimBar = false;
    if ( isAnimationActive() && frameCount() > 1 ) showAnimBar = true;

    if ( m_showInfoText ) columnWidth = std::max( columnWidth, m_infoLabel->sizeHint().width() );

    int columnPos = width() - columnWidth - margin - edgeAxisFrameBorderWidth;

    if ( isComparisonViewActive() )
    {
        Rim3dView* compView = dynamic_cast<Rim3dView*>( m_rimView.p() )->activeComparisonView();
        if ( compView )
        {
            columnWidth = 200;

            // int sliderPos = width() * comparisonViewVisibleNormalizedRect().min().x();
            int sliderPos         = 0.5 * width();
            int compViewItemsXPos = sliderPos + 0.5 * ( width() - sliderPos ) - 0.5 * columnWidth;
            columnPos             = 0.5 * sliderPos - 0.5 * columnWidth;

            if ( m_showInfoText )
            {
                {
                    Rim3dView* view = dynamic_cast<Rim3dView*>( m_rimView.p() );
                    m_shortInfoLabel->setText( "<center>" + view->ownerCase()->caseUserDescription() + "</center>" );

                    QPoint topLeft = QPoint( columnPos, yPos );
                    m_shortInfoLabel->resize( columnWidth, m_shortInfoLabel->sizeHint().height() );
                    m_shortInfoLabel->render( painter, topLeft );
                }

                {
                    m_shortInfoLabelCompView->setText( "<center>" + compView->ownerCase()->caseUserDescription() + "</center>" );
                    QPoint topLeft = QPoint( compViewItemsXPos, yPos );
                    m_shortInfoLabelCompView->resize( columnWidth, m_shortInfoLabelCompView->sizeHint().height() );
                    m_shortInfoLabelCompView->render( painter, topLeft );
                }

                yPos += m_shortInfoLabel->height();
            }

            int pickAreaHeight = yPos - startYPos;
            if ( m_showAnimProgress && isAnimationActive( true ) && compView->timeStepCount() > 1 )
            {
                QString stepName = compView->timeStepName( compView->currentTimeStep() );

                m_animationProgressCompView->setFormat( "Time Step: %v/%m " + stepName );
                m_animationProgressCompView->setMinimum( 0 );
                m_animationProgressCompView->setMaximum( static_cast<int>( compView->timeStepCount() ) - 1 );
                m_animationProgressCompView->setValue( compView->currentTimeStep() );

                m_animationProgressCompView->resize( columnWidth, m_animationProgressCompView->sizeHint().height() );

                m_animationProgressCompView->render( painter, QPoint( compViewItemsXPos, yPos ) );

                pickAreaHeight += m_animationProgressCompView->height();
            }

            m_infoPickArea.setLeft( columnPos );
            m_infoPickArea.setWidth( columnWidth );
            m_infoPickArea.setHeight( pickAreaHeight );
            m_infoPickArea.setTop( startYPos );

            m_infoPickAreaCompView.setLeft( compViewItemsXPos );
            m_infoPickAreaCompView.setWidth( columnWidth );
            m_infoPickAreaCompView.setHeight( pickAreaHeight );
            m_infoPickAreaCompView.setTop( startYPos );
        }
    }

    if ( showAnimBar && m_showAnimProgress )
    {
        Rim3dView* view = dynamic_cast<Rim3dView*>( m_rimView.p() );

        if ( view )
        {
            QString stepName = view->timeStepName( view->currentTimeStep() );

            m_animationProgress->setFormat( "Time Step: %v/%m " + stepName );
            m_animationProgress->setMinimum( 0 );
            m_animationProgress->setMaximum( static_cast<int>( view->timeStepCount() ) - 1 );
            m_animationProgress->setValue( view->currentTimeStep() );

            m_animationProgress->resize( columnWidth, m_animationProgress->sizeHint().height() );
            m_animationProgress->render( painter, QPoint( columnPos, yPos ) );

            yPos += m_animationProgress->height() + margin;
        }
    }

    if ( m_showInfoText && !isComparisonViewActive() )
    {
        QPoint topLeft = QPoint( columnPos, yPos );
        m_infoLabel->resize( columnWidth, m_infoLabel->sizeHint().height() );
        m_infoLabel->render( painter, topLeft );

        m_infoPickArea.setTopLeft( topLeft );
        m_infoPickArea.setBottom( yPos + m_infoLabel->height() );
        m_infoPickArea.setRight( columnPos + columnWidth );

        yPos += m_infoLabel->height() + margin;
    }
    else if ( !isComparisonViewActive() )
    {
        m_infoPickArea = QRect();
    }

    if ( m_showHistogram && !isComparisonViewActive() )
    {
        m_histogramWidget->resize( columnWidth, 40 );
        m_histogramWidget->render( painter, QPoint( columnPos, yPos ) );
        // yPos +=  m_histogramWidget->height() + margin;
    }

    if ( m_showVersionInfo ) // Version Label
    {
        QSize size( m_versionInfoLabel->sizeHint().width(), m_versionInfoLabel->sizeHint().height() );
        QPoint pos( width() - size.width() - margin - edgeAxisFrameBorderWidth, height() - size.height() - margin - edgeAxisFrameBorderHeight );
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

                painter->drawLine( centerPos.x(), centerPos.y() - markerHalfLength, centerPos.x(), centerPos.y() + markerHalfLength );
                painter->drawLine( centerPos.x() - markerHalfLength, centerPos.y(), centerPos.x() + markerHalfLength, centerPos.y() );
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
QString RiuViewer::infoText() const
{
    return m_infoLabel->text();
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
    bool isScaleChanged = m_zScale != scale;
    m_zScale            = scale;

    m_zScaleLabel->setText( QString( "Z: %1" ).arg( scale ) );

    if ( isScaleChanged ) m_selectionVisualizerManager->updateVisibleEditors();
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
    if ( enable )
    {
        addStaticModelOnce( m_gridBoxGenerator->model(), false );
        addStaticModelOnce( m_comparisonGridBoxGenerator->model(), true );
    }
    else
    {
        removeStaticModel( m_gridBoxGenerator->model() );
        removeStaticModel( m_comparisonGridBoxGenerator->model() );
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
void RiuViewer::mouseDoubleClickEvent( QMouseEvent* event )
{
    if ( auto view = dynamic_cast<Rim3dView*>( m_rimView.p() ) )
    {
        view->zoomAll();

        return;
    }

    caf::Viewer::mouseDoubleClickEvent( event );
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
    for ( auto legend : m_visibleComparisonLegends )
    {
        overlayItemsRendering()->removeOverlayItem( legend.p() );
    }

    m_visibleLegends.clear();
    m_visibleComparisonLegends.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::addColorLegendToBottomLeftCorner( caf::TitledOverlayFrame* addedLegend, bool isForComparisonView )
{
    if ( !addedLegend || m_visibleLegends.contains( addedLegend ) ) return;

    RiaGuiApplication* app              = RiaGuiApplication::instance();
    cvf::Rendering*    overlayRendering = overlayItemsRendering();
    CVF_ASSERT( overlayRendering );

    cvf::Color4f backgroundColor = mainCamera()->viewport()->clearColor();
    backgroundColor.a()          = 0.8f;

    cvf::Color3f backgroundColor3f( backgroundColor.r(), backgroundColor.g(), backgroundColor.b() );
    cvf::Color4f frameColor = cvf::Color4f( RiaColorTools::computeOffsetColor( backgroundColor3f, 0.3f ), 0.9f );

    updateLegendTextAndTickMarkColor( addedLegend );

    addedLegend->enableBackground( RiaPreferences::current()->showLegendBackground() );
    addedLegend->setBackgroundColor( backgroundColor );
    addedLegend->setBackgroundFrameColor( frameColor );
    addedLegend->setFont( app->sceneFont( m_fontPointSize ) );

    overlayRendering->addOverlayItem( addedLegend );

    if ( isForComparisonView )
    {
        m_visibleComparisonLegends.push_back( addedLegend );
    }
    else
    {
        m_visibleLegends.push_back( addedLegend );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::removeColorLegend( caf::TitledOverlayFrame* legend )
{
    overlayItemsRendering()->removeOverlayItem( legend );
    m_visibleLegends.erase( legend );
    m_visibleComparisonLegends.erase( legend );
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

    // Place the main view legends from left to right
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
                legend->setRenderSize( cvf::Vec2ui( legendWidth, std::max( 0, viewPortHeight - 2 * border - 2 * edgeAxisBorderHeight ) ) );
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

    // Place the comparison view legends from right to left
    {
        int viewPortWidth = static_cast<int>( m_mainCamera->viewport()->width() );

        int xPos      = viewPortWidth - border + edgeAxisBorderWidth;
        int yPosStart = border + edgeAxisBorderHeight + m_versionInfoLabel->sizeHint().height() + 5;
        int yPos      = yPosStart;

        std::vector<caf::TitledOverlayFrame*> standardHeightLegends;

        // Place the legends needing the full height, and sort out the standard height legends

        for ( cvf::ref<caf::TitledOverlayFrame> legend : m_visibleComparisonLegends )
        {
            cvf::Vec2ui prefSize = legend->preferredSize();
            if ( prefSize.y() > maxFreeLegendHeight )
            {
                int legendWidth = prefSize.x();
                legend->setLayoutFixedPosition( cvf::Vec2i( xPos - legendWidth, yPos ) );
                legend->setRenderSize( cvf::Vec2ui( legendWidth, std::max( 0, viewPortHeight - yPosStart - border - edgeAxisBorderHeight ) ) );
                xPos -= legendWidth + border;
            }
            else
            {
                standardHeightLegends.push_back( legend.p() );
            }
        }

        // Place the rest of the legends in columns that fits within the screen height

        std::vector<caf::TitledOverlayFrame*> columnLegends;

        int maxColumnWidht = 0;

        for ( caf::TitledOverlayFrame* legend : standardHeightLegends )
        {
            cvf::Vec2ui prefSize = legend->preferredSize();

            // Check if we need a new column
            if ( ( yPos + (int)prefSize.y() + border ) > viewPortHeight )
            {
                // Finish the previous column setting same width to all legends and correcting the xposition accordingly
                for ( caf::TitledOverlayFrame* columnLegend : columnLegends )
                {
                    columnLegend->setRenderSize( cvf::Vec2ui( maxColumnWidht, columnLegend->renderSize().y() ) );
                    columnLegend->setLayoutFixedPosition( cvf::Vec2i( xPos - maxColumnWidht, columnLegend->fixedPosition().y() ) );
                }

                // Increment to make ready for a new column
                xPos -= border + maxColumnWidht;
                yPos = yPosStart;

                maxColumnWidht = 0;
                columnLegends.clear();
            }

            legend->setLayoutFixedPosition( cvf::Vec2i( xPos - prefSize.x(), yPos ) );
            legend->setRenderSize( cvf::Vec2ui( prefSize.x(), prefSize.y() ) );
            columnLegends.push_back( legend );

            yPos += legend->renderSize().y() + border;
            maxColumnWidht = std::max( maxColumnWidht, (int)prefSize.x() );
        }

        // Finish the last column setting same width to all legends and correcting the xposition accordingly

        for ( caf::TitledOverlayFrame* columnLegend : columnLegends )
        {
            columnLegend->setRenderSize( cvf::Vec2ui( maxColumnWidht, columnLegend->renderSize().y() ) );
            columnLegend->setLayoutFixedPosition( cvf::Vec2i( xPos - maxColumnWidht, columnLegend->fixedPosition().y() ) );
        }

        xPos -= maxColumnWidht;

        // Set axis cross position at the bottom besides the last column
        {
            m_axisCross->setLayoutFixedPosition( cvf::Vec2i( xPos + border - m_axisCross->sizeHint().x(), edgeAxisBorderHeight ) );
        }
    }

    // Set the position of the scale bar used in contour map views
    {
        int  margin           = 5;
        auto scaleLegendSize  = m_scaleLegend->renderSize();
        auto otherItemsHeight = m_versionInfoLabel->sizeHint().height();

        const int xPos = width() - (int)scaleLegendSize.x() - margin - edgeAxisBorderWidth;
        const int yPos = margin + edgeAxisBorderHeight + margin + otherItemsHeight;

        m_scaleLegend->setLayoutFixedPosition( { xPos, yPos } );
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
        case RiaDefines::RINavigationPolicy::NAVIGATION_POLICY_CAD:
            setNavigationPolicy( new RiuCadNavigation );
            break;

        case RiaDefines::RINavigationPolicy::NAVIGATION_POLICY_CEETRON:
            setNavigationPolicy( new caf::CeetronPlusNavigation );
            break;

        case RiaDefines::RINavigationPolicy::NAVIGATION_POLICY_GEOQUEST:
            setNavigationPolicy( new RiuGeoQuestNavigation );
            break;

        case RiaDefines::RINavigationPolicy::NAVIGATION_POLICY_RMS:
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
            viewLinker->updateCamera( dynamic_cast<Rim3dView*>( m_rimView.p() ) );
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
    if ( m_rimView == nullptr ) return;

    if ( m_showWindowEdgeAxes )
    {
        m_windowEdgeAxisOverlay->setDisplayCoordTransform( m_rimView->displayCoordTransform().p() );
        m_windowEdgeAxisOverlay->updateFromCamera( mainCamera() );
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
            if ( mainCamera()->unproject( cvf::Vec3d( static_cast<double>( translatedMousePosX ), static_cast<double>( translatedMousePosY ), 0 ),
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

                auto view = dynamic_cast<Rim3dView*>( m_rimView.p() );
                if ( view ) viewLinker->updateCursorPosition( view, domainCoord );
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

        auto view = dynamic_cast<Rim3dView*>( m_rimView.p() );
        if ( view ) viewLinker->updateCursorPosition( view, cvf::Vec3d::UNDEFINED );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::updateGridBoxData( double                  scaleZ,
                                   const cvf::Vec3d&       displayModelOffset,
                                   const cvf::Color3f&     backgroundColor,
                                   const cvf::BoundingBox& domainCoordBoundingBox,
                                   int                     fontPointSize )
{
    m_gridBoxGenerator->setScaleZ( scaleZ );
    m_gridBoxGenerator->setDisplayModelOffset( displayModelOffset );
    m_gridBoxGenerator->updateFromBackgroundColor( backgroundColor );
    m_gridBoxGenerator->setGridBoxDomainCoordBoundingBox( domainCoordBoundingBox );
    m_gridBoxGenerator->setGridLabelFontSize( fontPointSize );

    m_gridBoxGenerator->createGridBoxParts();

    m_comparisonGridBoxGenerator->setScaleZ( scaleZ );
    cvf::Vec3d unscaledComparisonOffset = comparisonViewEyePointOffset();

    unscaledComparisonOffset.z() /= scaleZ;

    m_comparisonGridBoxGenerator->setDisplayModelOffset( displayModelOffset - unscaledComparisonOffset );
    m_comparisonGridBoxGenerator->updateFromBackgroundColor( backgroundColor );
    m_comparisonGridBoxGenerator->setGridBoxDomainCoordBoundingBox( domainCoordBoundingBox );

    m_comparisonGridBoxGenerator->createGridBoxParts();
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
RiuViewerCommands* RiuViewer::viewerCommands() const
{
    return m_viewerCommands;
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
             overlayItem->pick( translatedMousePosX, translatedMousePosY, overlayItem->fixedPosition(), overlayItem->renderSize() ) )
        {
            return overlayItem.p();
        }
    }

    for ( auto overlayItem : m_visibleComparisonLegends )
    {
        if ( overlayItem->layoutScheme() == cvf::OverlayItem::FIXED_POSITION &&
             overlayItem->pick( translatedMousePosX, translatedMousePosY, overlayItem->fixedPosition(), overlayItem->renderSize() ) )
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
            m_scaleLegend->setRenderSize( { 280, 45 } );
        else
            m_scaleLegend->setRenderSize( { 50, 280 } );

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
void RiuViewer::updateFonts( int fontPointSize )
{
    m_fontPointSize = fontPointSize;

    auto       defaultFontSize = RiaApplication::instance()->preferences()->defaultSceneFontSize();
    cvf::Font* axisFont        = RiaGuiApplication::instance()->defaultSceneFont();
    QFont      font            = QApplication::font();
    font.setPixelSize( caf::FontTools::pointSizeToPixelSize( m_fontPointSize ) );

    if ( caf::FontTools::absolutePointSize( defaultFontSize ) != m_fontPointSize )
    {
        axisFont = RiaFontCache::getFont( m_fontPointSize ).p();
    }

    overlayItemsRendering()->removeOverlayItem( m_axisCross.p() );

    m_axisCross = new cvf::OverlayAxisCross( m_mainCamera.p(), axisFont );
    m_axisCross->setAxisLabels( "X", "Y", "Z" );
    m_axisCross->setLayout( cvf::OverlayItem::VERTICAL, cvf::OverlayItem::BOTTOM_RIGHT );
    overlayItemsRendering()->addOverlayItem( m_axisCross.p() );
    m_showAxisCross = true;

    m_zScaleLabel->setFont( font );
    m_infoLabel->setFont( font );
    m_shortInfoLabel->setFont( font );
    m_shortInfoLabelCompView->setFont( font );
    m_animationProgress->setFont( font );
    m_animationProgressCompView->setFont( font );
    m_versionInfoLabel->setFont( font );

    m_gridBoxGenerator->setGridLabelFontSize( m_fontPointSize );
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

    for ( auto legend : m_visibleComparisonLegends )
    {
        updateLegendTextAndTickMarkColor( legend.p() );
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

    m_infoLabel->setStyleSheet(
        caf::StyleSheetTools::createFrameStyleSheet( "QLabel", "InfoLabel", contrastColor, backgroundColor, backgroundFrameColor ) );
    m_shortInfoLabel->setStyleSheet(
        caf::StyleSheetTools::createFrameStyleSheet( "QLabel", "ShortInfoLabel", contrastColor, backgroundColor, backgroundFrameColor ) );
    m_shortInfoLabelCompView->setStyleSheet(
        caf::StyleSheetTools::createFrameStyleSheet( "QLabel", "ShortInfoLabelCompView", contrastColor, backgroundColor, backgroundFrameColor ) );
    m_versionInfoLabel->setStyleSheet(
        caf::StyleSheetTools::createFrameStyleSheet( "QLabel", "VersionInfo", contrastColor, backgroundColor, backgroundColor ) );
    m_zScaleLabel->setStyleSheet(
        caf::StyleSheetTools::createFrameStyleSheet( "QLabel", "ZScaleLabel", contrastColor, backgroundColor, backgroundColor ) );
    m_histogramWidget->setStyleSheet(
        caf::StyleSheetTools::createFrameStyleSheet( "QWidget", "HistogramWidget", contrastColor, backgroundColor, backgroundFrameColor ) );

    QColor progressColor( Qt::green );
    progressColor.setAlphaF( 0.8f );
    backgroundColor.setAlphaF( 0.8f );
    m_animationProgress->setTextBackgroundAndProgressColor( contrastColor, backgroundColor, backgroundFrameColor, progressColor );
    m_animationProgressCompView->setTextBackgroundAndProgressColor( contrastColor, backgroundColor, backgroundFrameColor, progressColor );
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
