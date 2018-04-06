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

#pragma once

#include "RiuViewerToViewInterface.h"
#include "cafViewer.h"

#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafPdmInterfacePointer.h"

#include "cafMouseState.h"
#include "cvfStructGrid.h"
#include "RiuInterfaceToViewWindow.h"

class RicCommandFeature;
class Rim3dView;
class RiuSimpleHistogramWidget;
class RiuViewerCommands;
class RivGridBoxGenerator;
class RivWindowEdgeAxesOverlayItem;

class QCDEStyle;
class QLabel;
class QProgressBar;

namespace caf
{
    class TitledOverlayFrame;
}

namespace cvf
{
    class Color3f;
    class Model;
    class OverlayItem;
    class Part;
    class OverlayAxisCross;
    class BoundingBox;
}

//==================================================================================================
//
// RiuViewer
//
//==================================================================================================
class RiuViewer : public caf::Viewer, public RiuInterfaceToViewWindow
{
    Q_OBJECT

public:
    RiuViewer(const QGLFormat& format, QWidget* parent);
    ~RiuViewer();

    void            setDefaultView();
    cvf::Vec3d      pointOfInterest();
    void            setPointOfInterest(cvf::Vec3d poi);
    void            setOwnerReservoirView(RiuViewerToViewInterface * owner);
    RiuViewerToViewInterface*      ownerReservoirView();
    RimViewWindow*  ownerViewWindow() const override;
    void            setEnableMask(unsigned int mask);

    void            showInfoText(bool enable);
    void            setInfoText(QString text);
    void            showHistogram(bool enable);
    void            setHistogram(double min, double max, const std::vector<size_t>& histogram);
    void            setHistogramPercentiles(double pmin, double pmax, double mean);

    void            showGridBox(bool enable);
    void            updateGridBoxData(double scaleZ, 
                                      const cvf::Vec3d& displayModelOffset,
                                      const cvf::Color3f&  backgroundColor,
                                      const cvf::BoundingBox& domainCoordBoundingBox);
    void            showEdgeTickMarks(bool enable);

    void            updateAnnotationItems();

    void            showAnimationProgress(bool enable);
    
    void            removeAllColorLegends();
    void            addColorLegendToBottomLeftCorner(caf::TitledOverlayFrame* legend, const cvf::Color3f& backgroundColor);

    void            enableNavigationRotation(bool disable); 
    void            updateNavigationPolicy();

    virtual void    navigationPolicyUpdate();               // Override of caf::Viewer::navigationPolicyUpdate()

    void            setCurrentFrame(int frameIndex);

    void            showAxisCross(bool enable);
    void            setAxisLabels(const cvf::String& xLabel, const cvf::String& yLabel, const cvf::String& zLabel);

    cvf::Vec3d      lastPickPositionInDomainCoords() const;

    cvf::OverlayItem*   pickFixedPositionedLegend(int winPosX, int winPosY);

    void            updateParallelProjectionSettings(RiuViewer* sourceViewer);

    void            setCursorPosition(const cvf::Vec3d& domainCoord);

public slots:
    virtual void    slotSetCurrentFrame(int frameIndex);
    virtual void    slotEndAnimation();

protected:
    virtual void    optimizeClippingPlanes();
    virtual void    resizeGL(int width, int height);
    virtual void    mouseMoveEvent(QMouseEvent* e) override;
    virtual void    leaveEvent(QEvent *) override;

private:
    void            updateTextAndTickMarkColorForOverlayItems();
    void            updateLegendTextAndTickMarkColor(cvf::OverlayItem* legend);

    cvf::Color3f    computeContrastColor() const;

    void            updateAxisCrossTextColor();

    void            paintOverlayItems(QPainter* painter);

    void            mouseReleaseEvent(QMouseEvent* event);
    void            mousePressEvent(QMouseEvent* event);

private:
    QLabel*         m_infoLabel;
    QRect           m_infoLabelOverlayArea;

    QLabel*         m_versionInfoLabel;
    bool            m_showInfoText; 

    QProgressBar*   m_animationProgress;
    bool            m_showAnimProgress; 
    RiuSimpleHistogramWidget* m_histogramWidget;
    bool            m_showHistogram;

    QCDEStyle*      m_progressBarStyle;

    cvf::ref<cvf::OverlayAxisCross> m_axisCross;
    bool                            m_showAxisCross;
    cvf::Collection<caf::TitledOverlayFrame> m_visibleLegends;
    cvf::Collection<cvf::OverlayItem> allOverlayItems();


    caf::PdmInterfacePointer<RiuViewerToViewInterface>    m_rimView;
    QPoint                      m_lastMousePressPosition;

    RiuViewerCommands*          m_viewerCommands;

    RivGridBoxGenerator*        m_gridBoxGenerator;
    cvf::ref<RivWindowEdgeAxesOverlayItem> m_windowEdgeAxisOverlay;
    bool                        m_showWindowEdgeAxes;

    cvf::Vec3d                  m_cursorPositionDomainCoords;
    bool                        m_isNavigationRotationEnabled;
};

