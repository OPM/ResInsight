/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RimWellLogPlot.h"

#include "RimWellLogTrack.h"

#include "RiuWellLogPlot.h"
#include "RiuWellLogTrack.h"
#include "RiuMainWindow.h"

#include "cafPdmUiTreeView.h"

#include "cvfAssert.h"

#include <math.h>

#define RI_LOGPLOT_MINDEPTH_DEFAULT 0.0
#define RI_LOGPLOT_MAXDEPTH_DEFAULT 1000.0

namespace caf {

    template<>
    void caf::AppEnum< RimWellLogPlot::DepthTypeEnum >::setUp()
    {
        addItem(RimWellLogPlot::MEASURED_DEPTH,       "MEASURED_DEPTH",       "Measured Depth");
        addItem(RimWellLogPlot::TRUE_VERTICAL_DEPTH,  "TRUE_VERTICAL_DEPTH",  "True Vertical Depth");
        setDefault(RimWellLogPlot::MEASURED_DEPTH);
    }

} // End namespace caf


CAF_PDM_SOURCE_INIT(RimWellLogPlot, "WellLogPlot");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlot::RimWellLogPlot()
{
    CAF_PDM_InitObject("Well Log Plot", ":/WellLogPlot16x16.png", "", "");

    m_viewer = NULL;

    CAF_PDM_InitField(&m_showWindow, "ShowWindow", true, "Show well log plot", "", "", "");
    m_showWindow.uiCapability()->setUiHidden(true);
    
    CAF_PDM_InitField(&m_userName, "PlotDescription", QString("Well Log Plot"),"Name", "", "", "");
    
    caf::AppEnum< RimWellLogPlot::DepthTypeEnum > depthType = MEASURED_DEPTH;
    CAF_PDM_InitField(&m_depthType, "DepthType", depthType, "Depth type",   "", "", "");

    caf::AppEnum< RimDefines::DepthUnitType > depthUnit = RimDefines::UNIT_METER;
    CAF_PDM_InitField(&m_depthUnit, "DepthUnit", depthUnit, "Depth unit", "", "", "");

    CAF_PDM_InitField(&m_minVisibleDepth, "MinimumDepth", 0.0, "Min", "", "", "");
    CAF_PDM_InitField(&m_maxVisibleDepth, "MaximumDepth", 1000.0, "Max", "", "", "");    
    CAF_PDM_InitField(&m_isAutoScaleDepthEnabled, "AutoScaleDepthEnabled", true, "Auto Scale", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_tracks, "Tracks", "",  "", "", "");
    m_tracks.uiCapability()->setUiHidden(true);

    m_minAvailableDepth = HUGE_VAL;
    m_maxAvailableDepth = -HUGE_VAL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlot::~RimWellLogPlot()
{
    RiuMainWindow::instance()->removeViewer(m_viewer);
    
    detachAllCurves();
    m_tracks.deleteAllChildObjects();

    delete m_viewer;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::updateViewerWidget()
{
    if (m_showWindow())
    {
        if (!m_viewer)
        {
            m_viewer = new RiuWellLogPlot(this, RiuMainWindow::instance());

            recreateTrackPlots();

            RiuMainWindow::instance()->addViewer(m_viewer, this->mdiWindowGeometry());
            RiuMainWindow::instance()->setActiveViewer(m_viewer);
        }

        updateViewerWidgetWindowTitle();
    }
    else
    {
        if (m_viewer)
        {
            this->setMdiWindowGeometry( RiuMainWindow::instance()->windowGeometryForViewer(m_viewer));

            RiuMainWindow::instance()->removeViewer(m_viewer);
            detachAllCurves();

            delete m_viewer;
            m_viewer = NULL;
           
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_showWindow)
    {
        if (m_showWindow)
        {
            loadDataAndUpdate();
        }
        else
        {
            updateViewerWidget();
        }

        uiCapability()->updateUiIconFromToggleField();
    }
    else if (changedField == &m_minVisibleDepth || changedField == &m_maxVisibleDepth)
    {
        applyDepthZoomFromVisibleDepth();

        m_isAutoScaleDepthEnabled = false;
    }
    else if (changedField == &m_isAutoScaleDepthEnabled)
    {
        updateDepthZoom();
    }
    else if (changedField == &m_userName)
    {
        updateViewerWidgetWindowTitle();
    }
    if (changedField == &m_depthType ||
        changedField == &m_depthUnit)
    {
        updateTracks();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWellLogPlot::objectToggleField()
{
    return &m_showWindow;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::addTrack(RimWellLogTrack* track)
{
    m_tracks.push_back(track);
    if (m_viewer)
    {
        track->recreateViewer();
        m_viewer->addTrackPlot(track->viewer());
    }

    updateTrackNames();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::insertTrack(RimWellLogTrack* track, size_t index)
{
    m_tracks.insert(index, track);

    if (m_viewer)
    {
        track->recreateViewer();
        m_viewer->insertTrackPlot(track->viewer(), index);
    }

    updateTrackNames();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::removeTrack(RimWellLogTrack* track)
{
    if (track)
    {
        if (m_viewer) m_viewer->removeTrackPlot(track->viewer());
        m_tracks.removeChildObject(track);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::moveTracks(RimWellLogTrack* insertAfterTrack, const std::vector<RimWellLogTrack*>& tracksToMove)
{
    for (size_t tIdx = 0; tIdx < tracksToMove.size(); tIdx++)
    {
        RimWellLogTrack* track = tracksToMove[tIdx];

        RimWellLogPlot* wellLogPlot;
        track->firstAnchestorOrThisOfType(wellLogPlot);
        if (wellLogPlot)
        {
            wellLogPlot->removeTrack(track);
            wellLogPlot->updateTrackNames();
            wellLogPlot->updateConnectedEditors();
        }
    }

    size_t index = m_tracks.index(insertAfterTrack) + 1;

    for (size_t tIdx = 0; tIdx < tracksToMove.size(); tIdx++)
    {
        insertTrack(tracksToMove[tIdx], index + tIdx);
    }

    updateTrackNames();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuWellLogPlot* RimWellLogPlot::viewer()
{
    return m_viewer;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::setDepthZoomByFactorAndCenter(double zoomFactor, double zoomCenter)
{
    double newMinimum = zoomCenter - (zoomCenter - m_minVisibleDepth)*zoomFactor;
    double newMaximum = zoomCenter + (m_maxVisibleDepth - zoomCenter)*zoomFactor;

    setDepthZoomMinMax(newMinimum, newMaximum);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::panDepth(double panFactor)
{
    double delta = panFactor*(m_maxVisibleDepth - m_minVisibleDepth);
    setDepthZoomMinMax(m_minVisibleDepth + delta, m_maxVisibleDepth + delta);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::setDepthZoomMinMax(double minimumDepth, double maximumDepth)
{
    m_minVisibleDepth = minimumDepth;
    m_maxVisibleDepth = maximumDepth;

    m_minVisibleDepth.uiCapability()->updateConnectedEditors();
    m_maxVisibleDepth.uiCapability()->updateConnectedEditors();

    applyDepthZoomFromVisibleDepth();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::calculateAvailableDepthRange()
{
    double minDepth = HUGE_VAL;
    double maxDepth = -HUGE_VAL;

    for (size_t tIdx = 0; tIdx < m_tracks.size(); tIdx++)
    {
        double minTrackDepth = HUGE_VAL;
        double maxTrackDepth = -HUGE_VAL;

        if (m_tracks[tIdx]->isVisible())
        {
            m_tracks[tIdx]->availableDepthRange(&minTrackDepth, &maxTrackDepth);

            if (minTrackDepth < minDepth)
            {
                minDepth = minTrackDepth;
            }

            if (maxTrackDepth > maxDepth)
            {
                maxDepth = maxTrackDepth;
            }
        }
    }

    m_minAvailableDepth = minDepth;
    m_maxAvailableDepth = maxDepth;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::availableDepthRange(double* minimumDepth, double* maximumDepth) const
{
    if (hasAvailableDepthRange())
    {
        *minimumDepth = m_minAvailableDepth;
        *maximumDepth = m_maxAvailableDepth;
    }
    else
    {
        *minimumDepth = RI_LOGPLOT_MINDEPTH_DEFAULT;
        *maximumDepth = RI_LOGPLOT_MAXDEPTH_DEFAULT;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellLogPlot::hasAvailableDepthRange() const
{
    return m_minAvailableDepth < HUGE_VAL && m_maxAvailableDepth > -HUGE_VAL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::depthZoomMinMax(double* minimumDepth, double* maximumDepth) const
{
    *minimumDepth = m_minVisibleDepth;
    *maximumDepth = m_maxVisibleDepth;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::setupBeforeSave()
{
    if (m_viewer)
    {
        this->setMdiWindowGeometry( RiuMainWindow::instance()->windowGeometryForViewer(m_viewer));
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_userName);
    uiOrdering.add(&m_depthType);
    uiOrdering.add(&m_depthUnit);

    caf::PdmUiGroup* gridGroup = uiOrdering.addNewGroup("Visible Depth Range");
    gridGroup->add(&m_isAutoScaleDepthEnabled);
    gridGroup->add(&m_minVisibleDepth);
    gridGroup->add(&m_maxVisibleDepth);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::loadDataAndUpdate()
{
    updateViewerWidget();
    updateTracks();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::updateTracks()
{
    if (m_showWindow)
    {
        for (size_t tIdx = 0; tIdx < m_tracks.size(); ++tIdx)
        {
            m_tracks[tIdx]->loadDataAndUpdate();
        }

        calculateAvailableDepthRange();
        updateDepthZoom();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::updateTrackNames()
{
    for (size_t tIdx = 0; tIdx < m_tracks.size(); tIdx++)
    {
        m_tracks[tIdx]->setDescription(QString("Track %1").arg(tIdx + 1));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::updateDepthZoom()
{
    if (m_isAutoScaleDepthEnabled)
    {
        applyZoomAllDepths();
    }
    else
    {
        applyDepthZoomFromVisibleDepth();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::applyDepthZoomFromVisibleDepth()
{
    if (m_viewer)
    {
        double minDepth = m_minVisibleDepth < HUGE_VAL ? m_minVisibleDepth : RI_LOGPLOT_MINDEPTH_DEFAULT;
        double maxDepth = m_maxVisibleDepth > -HUGE_VAL ? m_maxVisibleDepth : RI_LOGPLOT_MAXDEPTH_DEFAULT;

        m_viewer->setDepthZoomAndReplot(minDepth, maxDepth);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::applyZoomAllDepths()
{
    if (hasAvailableDepthRange())
    {
        setDepthZoomMinMax(m_minAvailableDepth, m_maxAvailableDepth + 0.01*(m_maxAvailableDepth - m_minAvailableDepth));
    }
    else
    {
        setDepthZoomMinMax(RI_LOGPLOT_MINDEPTH_DEFAULT, RI_LOGPLOT_MAXDEPTH_DEFAULT);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::recreateTrackPlots()
{
    CVF_ASSERT(m_viewer);

    for (size_t tIdx = 0; tIdx < m_tracks.size(); ++tIdx)
    {
        m_tracks[tIdx]->recreateViewer();
        m_viewer->addTrackPlot(m_tracks[tIdx]->viewer());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::detachAllCurves()
{
    for (size_t tIdx = 0; tIdx < m_tracks.size(); ++tIdx)
    {
       m_tracks[tIdx]->detachAllCurves();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::setDescription(const QString& description)
{
    m_userName = description;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::updateViewerWidgetWindowTitle()
{
    if (m_viewer)
    {
        m_viewer->setWindowTitle(m_userName);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::handleViewerDeletion()
{
    m_showWindow = false;

    if (m_viewer)
    {
        detachAllCurves();
        m_viewer = NULL;
    }
 
    uiCapability()->updateUiIconFromToggleField();
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlot::DepthTypeEnum RimWellLogPlot::depthType() const
{
    return m_depthType.value();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimDefines::DepthUnitType RimWellLogPlot::depthUnit() const
{
    return m_depthUnit.value();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellLogPlot::depthPlotTitle() const
{
    QString depthTitle = "Depth";
    
    switch (m_depthType.value())
    {
        case MEASURED_DEPTH:
            depthTitle = "MD";
            break;

        case TRUE_VERTICAL_DEPTH:
            depthTitle = "TVD";
            break;
    }

    if (m_depthUnit == RimDefines::UNIT_METER)
    {
        depthTitle += " [m]";
    }
    else if (m_depthUnit == RimDefines::UNIT_FEET)
    {
        depthTitle += " [ft]";
    }

    return depthTitle;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RimWellLogPlot::trackIndex(RimWellLogTrack* track)
{
    return m_tracks.index(track);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::setDepthUnit(RimDefines::DepthUnitType depthUnit)
{
    m_depthUnit = depthUnit;

    updateTracks();
}

