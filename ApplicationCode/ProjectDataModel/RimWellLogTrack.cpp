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

#include "RimWellLogTrack.h"

#include "RiaApplication.h"
#include "RiaColorTables.h"
#include "RiaExtractionTools.h"
#include "RiaSimWellBranchTools.h"

#include "RigEclipseCaseData.h"
#include "RigEclipseWellLogExtractor.h"
#include "RigFemPartResultsCollection.h"
#include "RigFemResultAddress.h"
#include "RigFormationNames.h"
#include "RigGeoMechCaseData.h"
#include "RigGeoMechWellLogExtractor.h"
#include "RigResultAccessorFactory.h"
#include "RigSimWellData.h"
#include "RigSimulationWellCenterLineCalculator.h"
#include "RigSimulationWellCoordsAndMD.h"
#include "RigStatisticsCalculator.h"
#include "RigWellLogCurveData.h"
#include "RigWellPath.h"
#include "RigWellPathFormations.h"

#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimGeoMechCase.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimTools.h"
#include "RimWellPathAttribute.h"
#include "RimWellPathAttributeCurve.h"
#include "RimWellPathAttributeCollection.h"
#include "RimWellFlowRateCurve.h"
#include "RimWellLogCurve.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellPath.h"
#include "RimWellPltPlot.h"
#include "RimWellRftPlot.h"

#include "RiuMainWindow.h"
#include "RiuPlotAnnotationTool.h"
#include "RiuWellLogPlot.h"
#include "RiuWellLogTrack.h"

#include "RiuQwtLinearScaleEngine.h"
#include "cvfAssert.h"

#define RI_LOGPLOTTRACK_MINX_DEFAULT    -10.0
#define RI_LOGPLOTTRACK_MAXX_DEFAULT    100.0
#define RI_LOGPLOTTRACK_MINOR_TICK_DEFAULT 

CAF_PDM_SOURCE_INIT(RimWellLogTrack, "WellLogPlotTrack");

namespace caf
{
    template<>
    void AppEnum< RimWellLogTrack::TrajectoryType >::setUp()
    {
        addItem(RimWellLogTrack::WELL_PATH, "WELL_PATH", "Well Path");
        addItem(RimWellLogTrack::SIMULATION_WELL, "SIMULATION_WELL", "Simulation Well");
        setDefault(RimWellLogTrack::WELL_PATH);
    }

    template<>
    void AppEnum< RimWellLogTrack::FormationSource >::setUp()
    {
        addItem(RimWellLogTrack::CASE, "CASE", "Case");
        addItem(RimWellLogTrack::WELL_PICK_FILTER, "WELL_PICK_FILTER", "Well Picks for Well Path");
        setDefault(RimWellLogTrack::CASE);
    }
   
    template<>
    void AppEnum<RigWellPathFormations::FormationLevel>::setUp()
    {
        addItem(RigWellPathFormations::NONE, "NONE", "None");
        addItem(RigWellPathFormations::ALL, "ALL", "All");
        addItem(RigWellPathFormations::GROUP, "GROUP", "Formation Group");
        addItem(RigWellPathFormations::LEVEL0, "LEVEL0", "Formation");
        addItem(RigWellPathFormations::LEVEL1, "LEVEL1", "Formation 1");
        addItem(RigWellPathFormations::LEVEL2, "LEVEL2", "Formation 2");
        addItem(RigWellPathFormations::LEVEL3, "LEVEL3", "Formation 3");
        addItem(RigWellPathFormations::LEVEL4, "LEVEL4", "Formation 4");
        addItem(RigWellPathFormations::LEVEL5, "LEVEL5", "Formation 5");
        addItem(RigWellPathFormations::LEVEL6, "LEVEL6", "Formation 6");
        addItem(RigWellPathFormations::LEVEL7, "LEVEL7", "Formation 7");
        addItem(RigWellPathFormations::LEVEL8, "LEVEL8", "Formation 8");
        addItem(RigWellPathFormations::LEVEL9, "LEVEL9", "Formation 9");
        addItem(RigWellPathFormations::LEVEL10, "LEVEL10", "Formation 10");
        setDefault(RigWellPathFormations::ALL);
    }

    template<>
    void AppEnum< RimWellLogTrack::WidthScaleFactor >::setUp()
    {
        addItem(RimWellLogTrack::EXTRA_NARROW_TRACK, "EXTRA_NARROW_TRACK", "Extra Narrow");
        addItem(RimWellLogTrack::NARROW_TRACK,       "NARROW_TRACK",       "Narrow");
        addItem(RimWellLogTrack::NORMAL_TRACK,       "NORMAL_TRACK",       "Normal");
        addItem(RimWellLogTrack::WIDE_TRACK,         "WIDE_TRACK",         "Wide");
        addItem(RimWellLogTrack::EXTRA_WIDE_TRACK,    "EXTRA_WIDE_TRACK",  "Extra wide");
        setDefault(RimWellLogTrack::NORMAL_TRACK);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogTrack::RimWellLogTrack()
{
    CAF_PDM_InitObject("Track", ":/WellLogTrack16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_userName, "TrackDescription", "Name", "", "", "");
    m_userName.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitField(&m_show, "Show", true, "Show Track", "", "", "");
    m_show.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&curves, "Curves", "",  "", "", "");
    curves.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_visibleXRangeMin, "VisibleXRangeMin", RI_LOGPLOTTRACK_MINX_DEFAULT, "Min", "", "", "");
    CAF_PDM_InitField(&m_visibleXRangeMax, "VisibleXRangeMax", RI_LOGPLOTTRACK_MAXX_DEFAULT, "Max", "", "", "");

    CAF_PDM_InitField(&m_isAutoScaleXEnabled, "AutoScaleX", true, "Auto Scale", "", "", "");
    m_isAutoScaleXEnabled.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_isLogarithmicScaleEnabled, "LogarithmicScaleX", false, "Logarithmic Scale", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_xAxisGridVisibility, "ShowXGridLines", "Show Grid Lines", "", "", "");

    CAF_PDM_InitField(&m_explicitTickIntervals, "ExplicitTickIntervals", false, "Manually Set Tick Intervals", "", "", "");
    CAF_PDM_InitField(&m_majorTickInterval, "MajorTickIntervals", 0.0, "Major Tick Interval", "", "", "");
    CAF_PDM_InitField(&m_minorTickInterval, "MinorTickIntervals", 0.0, "Minor Tick Interval", "", "", "");
    m_majorTickInterval.uiCapability()->setUiHidden(true);
    m_minorTickInterval.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_showFormations, "ShowFormations", false, "Show Lines", "", "", "");
    CAF_PDM_InitField(&m_showFormationLabels, "ShowFormationLabels", true, "Show Labels", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_formationSource, "FormationSource", "Source", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_formationTrajectoryType, "FormationTrajectoryType", "Trajectory", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_formationWellPathForSourceCase, "FormationWellPath", "Well Path", "", "", "");
    m_formationWellPathForSourceCase.uiCapability()->setUiTreeChildrenHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_formationWellPathForSourceWellPath, "FormationWellPathForSourceWellPath", "Well Path", "", "", "");
    m_formationWellPathForSourceWellPath.uiCapability()->setUiTreeChildrenHidden(true);

    CAF_PDM_InitField(&m_formationSimWellName, "FormationSimulationWellName", QString("None"), "Simulation Well", "", "", "");
    CAF_PDM_InitField(&m_formationBranchIndex, "FormationBranchIndex", 0, " ", "", "", "");
    CAF_PDM_InitField(&m_formationBranchDetection, "FormationBranchDetection", true, "Branch Detection", "", 
                      "Compute branches based on how simulation well cells are organized", "");

    CAF_PDM_InitFieldNoDefault(&m_formationCase, "FormationCase", "Formation Case", "", "", "");
    m_formationCase.uiCapability()->setUiTreeChildrenHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_formationLevel, "FormationLevel", "Well Pick Filter", "", "", "");

    CAF_PDM_InitField(&m_showformationFluids, "ShowFormationFluids", false, "Show Fluids", "", "", "");

    CAF_PDM_InitField(&m_showWellPathAttributes, "ShowWellPathAttributes", false, "Show Well Attributes", "", "", "");
    CAF_PDM_InitField(&m_showWellPathAttributeBothSides, "ShowWellPathAttrBothSides", true, "Show Both Sides", "", "", "");
    CAF_PDM_InitField(&m_wellPathAttributesInLegend, "WellPathAttributesInLegend", false, "Contribute to Legend", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_wellPathAttributeSource, "AttributesWellPathSource", "Well Path", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_wellPathAttributeCollection, "AttributesCollection", "Well Attributes", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_wellPathAttributeCurves, "AttributeCurves", "", "", "", "");
    m_wellPathAttributeCurves.uiCapability()->setUiHidden(true);
    m_wellPathAttributeCurves.uiCapability()->setUiTreeChildrenHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_widthScaleFactor, "Width", "Track Width", "", "Set width of track. ", "");

    m_formationsForCaseWithSimWellOnly = false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogTrack::~RimWellLogTrack()
{
    curves.deleteAllChildObjects();

    if (m_wellLogTrackPlotWidget) 
    {
        m_wellLogTrackPlotWidget->deleteLater();
        m_wellLogTrackPlotWidget = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setDescription(const QString& description)
{
    m_userName = description;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::simWellOptionItems(QList<caf::PdmOptionItemInfo>* options, RimCase* rimCase)
{
    CVF_ASSERT(options);
    if (!options) return;

    std::set<QString> sortedWellNames;

    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(rimCase);

    if (eclipseCase && eclipseCase->eclipseCaseData())
    {
        sortedWellNames = eclipseCase->eclipseCaseData()->findSortedWellNames();
    }

    QIcon simWellIcon(":/Well.png");
    for (const QString& wname : sortedWellNames)
    {
        options->push_back(caf::PdmOptionItemInfo(wname, wname, false, simWellIcon));
    }

    if (options->size() == 0)
    {
        options->push_front(caf::PdmOptionItemInfo("None", "None"));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_show)
    {
        if (m_wellLogTrackPlotWidget)
        {
            m_wellLogTrackPlotWidget->setVisible(m_show());
        }

        updateParentPlotLayout();
    }
    else if (changedField == &m_widthScaleFactor)
    {
        updateParentPlotLayout();
        updateAxisAndGridTickIntervals();
    }
    else if (changedField == &m_explicitTickIntervals)
    {
        if (m_wellLogTrackPlotWidget)
        {
            m_majorTickInterval = m_wellLogTrackPlotWidget->getCurrentMajorTickInterval();
            m_minorTickInterval = m_wellLogTrackPlotWidget->getCurrentMinorTickInterval();
        }
        m_majorTickInterval.uiCapability()->setUiHidden(!m_explicitTickIntervals());
        m_minorTickInterval.uiCapability()->setUiHidden(!m_explicitTickIntervals());
        if (!m_explicitTickIntervals())
        {
            updateAxisAndGridTickIntervals();
        }
    }
    else if (changedField == &m_xAxisGridVisibility ||
             changedField == &m_majorTickInterval ||
             changedField == &m_minorTickInterval)
    {
        updateAxisAndGridTickIntervals();
    }
    else if (changedField == &m_visibleXRangeMin || changedField == &m_visibleXRangeMax)
    {
        m_wellLogTrackPlotWidget->setXRange(m_visibleXRangeMin, m_visibleXRangeMax);
        m_wellLogTrackPlotWidget->replot();
        m_isAutoScaleXEnabled = false;
        bool emptyRange = std::abs(m_visibleXRangeMax() - m_visibleXRangeMin) < 1.0e-6 * std::max(1.0, std::max(m_visibleXRangeMax(), m_visibleXRangeMin()));
        m_explicitTickIntervals.uiCapability()->setUiReadOnly(emptyRange);
        m_xAxisGridVisibility.uiCapability()->setUiReadOnly(emptyRange);

        updateEditors();
        updateParentPlotLayout();
        updateAxisAndGridTickIntervals();
    }
    else if (changedField == &m_isAutoScaleXEnabled)
    {
        if (m_isAutoScaleXEnabled())
        { 
            this->calculateXZoomRangeAndUpdateQwt();
            computeAndSetXRangeMinForLogarithmicScale();

            if (m_wellLogTrackPlotWidget) m_wellLogTrackPlotWidget->replot();
        }    
    }
    else if (changedField == &m_isLogarithmicScaleEnabled)
    {
        updateAxisScaleEngine();
        if (m_isLogarithmicScaleEnabled())
        {
            m_explicitTickIntervals = false;
        }
        m_explicitTickIntervals.uiCapability()->setUiHidden(m_isLogarithmicScaleEnabled());

        this->calculateXZoomRangeAndUpdateQwt();
        computeAndSetXRangeMinForLogarithmicScale();

        m_wellLogTrackPlotWidget->setXRange(m_visibleXRangeMin, m_visibleXRangeMax);

        m_wellLogTrackPlotWidget->replot();
    }
    else if (changedField == &m_showFormations || changedField == &m_formationSource)
    {
        if (changedField == &m_formationSource && m_formationSource == WELL_PICK_FILTER)
        {
            std::vector<RimWellPath*> wellPaths;
            RimTools::wellPathWithFormations(&wellPaths);
            for (RimWellPath* wellPath : wellPaths)
            {
                if (wellPath == m_formationWellPathForSourceCase)
                {
                    m_formationWellPathForSourceWellPath = m_formationWellPathForSourceCase();
                    break;
                }
            }
        }

        loadDataAndUpdate();
        RimWellRftPlot* rftPlot(nullptr);

        firstAncestorOrThisOfType(rftPlot);

        if (rftPlot)
        {
            rftPlot->updateConnectedEditors();
        }
        else
        {
            RimWellPltPlot* pltPlot(nullptr);
            firstAncestorOrThisOfType(pltPlot);

            if (pltPlot)
            {
                pltPlot->updateConnectedEditors();
            }
        }
    }
    else if (changedField == &m_showFormationLabels)
    {
        loadDataAndUpdate();
    }
    else if (changedField == &m_formationCase)
    {
        QList<caf::PdmOptionItemInfo> options;
        RimWellLogTrack::simWellOptionItems(&options, m_formationCase);

        if (options.isEmpty() || m_formationCase == nullptr)
        {
            m_formationSimWellName = QString("None");
        }

        loadDataAndUpdate();
    }
    else if (changedField == &m_formationWellPathForSourceCase)
    {
        loadDataAndUpdate();
    }
    else if (changedField == &m_formationSimWellName)
    {
        loadDataAndUpdate();
    }
    else if (changedField == &m_formationTrajectoryType)
    {
        if (m_formationTrajectoryType == WELL_PATH)
        {
            RimProject* proj = RiaApplication::instance()->project();
            m_formationWellPathForSourceCase = proj->wellPathFromSimWellName(m_formationSimWellName);
        }
        else
        {
            if (m_formationWellPathForSourceCase)
            {
                m_formationSimWellName = m_formationWellPathForSourceCase->associatedSimulationWellName();
            }
        }

        loadDataAndUpdate();
    }
    else if (changedField == &m_formationBranchIndex || 
             changedField == &m_formationBranchDetection)
    {
        m_formationBranchIndex = RiaSimWellBranchTools::clampBranchIndex(m_formationSimWellName, m_formationBranchIndex, m_formationBranchDetection);

        loadDataAndUpdate();
    }
    else if (changedField == &m_formationWellPathForSourceWellPath)
    {
        loadDataAndUpdate();
    }
    else if (changedField == &m_formationLevel)
    {
        loadDataAndUpdate();
    }
    else if (changedField == &m_showformationFluids)
    {
        loadDataAndUpdate();
    }
    else if (changedField == &m_showWellPathAttributes || 
             changedField == &m_showWellPathAttributeBothSides ||
             changedField == &m_wellPathAttributesInLegend)
    {
        updateWellPathAttributesOnPlot();
    }
    else if (changedField == &m_wellPathAttributeSource)
    {      
        updateWellPathAttributesCollection();
        updateWellPathAttributesOnPlot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::updateParentPlotLayout()
{
    RimWellLogPlot* wellLogPlot;
    this->firstAncestorOrThisOfType(wellLogPlot);
    if (wellLogPlot)
    {
        RiuWellLogPlot* wellLogPlotViewer = dynamic_cast<RiuWellLogPlot*>(wellLogPlot->viewWidget());
        if (wellLogPlotViewer)
        {
            wellLogPlotViewer->updateChildrenLayout();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::updateAxisAndGridTickIntervals()
{
    if (!m_wellLogTrackPlotWidget) return;

    if (m_explicitTickIntervals)
    {
        m_wellLogTrackPlotWidget->setMajorAndMinorTickIntervals(m_majorTickInterval(), m_minorTickInterval());
    }
    else
    {
        int xMajorTickIntervals = 3;
        int xMinorTickIntervals = 0;
        switch (m_widthScaleFactor())
        {
        case EXTRA_NARROW_TRACK:
            xMajorTickIntervals = 3;
            xMinorTickIntervals = 2;
            break;
        case NARROW_TRACK:
            xMajorTickIntervals = 3;
            xMinorTickIntervals = 5;
            break;
        case NORMAL_TRACK:
            xMajorTickIntervals = 5;
            xMinorTickIntervals = 5;
            break;
        case WIDE_TRACK:
            xMajorTickIntervals = 5;
            xMinorTickIntervals = 10;
            break;
        case EXTRA_WIDE_TRACK:
            xMajorTickIntervals = 10;
            xMinorTickIntervals = 10;
            break;
        }
        m_wellLogTrackPlotWidget->setAutoTickIntervalCounts(xMajorTickIntervals, xMinorTickIntervals);
    }

    switch (m_xAxisGridVisibility())
    {
    case RimWellLogPlot::AXIS_GRID_NONE:
        m_wellLogTrackPlotWidget->enableXGridLines(false, false);
        break;
    case RimWellLogPlot::AXIS_GRID_MAJOR:
        m_wellLogTrackPlotWidget->enableXGridLines(true, false);
        break;
    case RimWellLogPlot::AXIS_GRID_MAJOR_AND_MINOR:
        m_wellLogTrackPlotWidget->enableXGridLines(true, true);
        break;
    }

    RimWellLogPlot* plot = nullptr;
    this->firstAncestorOrThisOfTypeAsserted(plot);
    switch (plot->depthGridLinesVisibility())
    {
    case RimWellLogPlot::AXIS_GRID_NONE:
        m_wellLogTrackPlotWidget->enableDepthGridLines(false, false);
        break;
    case RimWellLogPlot::AXIS_GRID_MAJOR:
        m_wellLogTrackPlotWidget->enableDepthGridLines(true, false);
        break;
    case RimWellLogPlot::AXIS_GRID_MAJOR_AND_MINOR:
        m_wellLogTrackPlotWidget->enableDepthGridLines(true, true);
        break;
    }
    m_wellLogTrackPlotWidget->replot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::updateAllLegendItems()
{
    reattachAllCurves();
    if (m_wellLogTrackPlotWidget)
    {
        m_wellLogTrackPlotWidget->updateLegend();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellLogTrack::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (options.size() > 0) return options;

    if (fieldNeedingOptions == &m_formationWellPathForSourceCase)
    {
        RimTools::wellPathOptionItems(&options);
        options.push_front(caf::PdmOptionItemInfo("None", nullptr));
    }
    else if (fieldNeedingOptions == &m_formationWellPathForSourceWellPath)
    {
        RimTools::wellPathWithFormationsOptionItems(&options);
        options.push_front(caf::PdmOptionItemInfo("None", nullptr));
    }
    else if (fieldNeedingOptions == &m_formationCase)
    {
        RimTools::caseOptionItems(&options);

        options.push_front(caf::PdmOptionItemInfo("None", nullptr));
    }
    else if (fieldNeedingOptions == &m_formationSimWellName)
    {
        RimWellLogTrack::simWellOptionItems(&options, m_formationCase);
    }
    else if (fieldNeedingOptions == &m_formationBranchIndex)
    {
        auto simulationWellBranches = RiaSimWellBranchTools::simulationWellBranches(m_formationSimWellName(), m_formationBranchDetection);
        options = RiaSimWellBranchTools::valueOptionsForBranchIndexField(simulationWellBranches);
    }
    else if (fieldNeedingOptions == &m_formationLevel)
    {
        if (m_formationWellPathForSourceWellPath)
        {
            const RigWellPathFormations* formations = m_formationWellPathForSourceWellPath->formationsGeometry();
            if (formations)
            {
                using FormationLevelEnum = caf::AppEnum<RigWellPathFormations::FormationLevel>;
                
                options.push_back(caf::PdmOptionItemInfo(FormationLevelEnum::uiText(RigWellPathFormations::NONE),
                                                         RigWellPathFormations::NONE));

                options.push_back(caf::PdmOptionItemInfo(FormationLevelEnum::uiText(RigWellPathFormations::ALL),
                                                         RigWellPathFormations::ALL));

                for (const RigWellPathFormations::FormationLevel& level : formations->formationsLevelsPresent())
                {
                    size_t index = FormationLevelEnum::index(level);
                    if (index >= FormationLevelEnum::size()) continue;

                    options.push_back(caf::PdmOptionItemInfo(FormationLevelEnum::uiTextFromIndex(index),
                                                             FormationLevelEnum::fromIndex(index)));
                }
            }
        }
    }
    else if (fieldNeedingOptions == &m_wellPathAttributeSource)
    {
        RimTools::wellPathOptionItems(&options);
        options.push_front(caf::PdmOptionItemInfo("None", nullptr));
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWellLogTrack::objectToggleField()
{
    return &m_show;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWellLogTrack::userDescriptionField()
{
    return &m_userName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::addCurve(RimWellLogCurve* curve)
{
    curves.push_back(curve);

    if (m_wellLogTrackPlotWidget)
    {
        curve->setParentQwtPlotAndReplot(m_wellLogTrackPlotWidget);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::insertCurve(RimWellLogCurve* curve, size_t index)
{
    curves.insert(index, curve);
    // Todo: Mark curve data to use either TVD or MD

    if (m_wellLogTrackPlotWidget)
    {
        curve->setParentQwtPlotAndReplot(m_wellLogTrackPlotWidget);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::takeOutCurve(RimWellLogCurve* curve)
{
    size_t index = curves.index(curve);
    if ( index < curves.size())
    {
        curves[index]->detachQwtCurve();
        curves.removeChildObject(curve);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::deleteAllCurves()
{
    curves.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuWellLogTrack* RimWellLogTrack::viewer()
{
    return m_wellLogTrackPlotWidget;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::availableDepthRange(double* minimumDepth, double* maximumDepth)
{
    double minDepth = HUGE_VAL;
    double maxDepth = -HUGE_VAL;


    std::vector<RimPlotCurve*> allCurves;
    allCurves.insert(allCurves.end(), curves.begin(), curves.end());
    allCurves.insert(allCurves.end(), m_wellPathAttributeCurves.begin(), m_wellPathAttributeCurves.end());

    for (RimPlotCurve* curve : allCurves)
    {
        double minCurveDepth = HUGE_VAL;
        double maxCurveDepth = -HUGE_VAL;

        if (curve->isCurveVisible() && curve->yValueRange(&minCurveDepth, &maxCurveDepth))
        {
            if (minCurveDepth < minDepth)
            {
                minDepth = minCurveDepth;
            }

            if (maxCurveDepth > maxDepth)
            {
                maxDepth = maxCurveDepth;
            }
        }
    }

    *minimumDepth = minDepth;
    *maximumDepth = maxDepth;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::loadDataAndUpdate()
{
    RimWellLogPlot* wellLogPlot = nullptr;
    firstAncestorOrThisOfType(wellLogPlot);

    if (wellLogPlot && m_wellLogTrackPlotWidget)
    {
        if (isFirstVisibleTrackInPlot())
        {
            m_wellLogTrackPlotWidget->setDepthTitle(wellLogPlot->depthPlotTitle());
        }
        m_wellLogTrackPlotWidget->setXTitle(m_xAxisTitle);
    }

    for (size_t cIdx = 0; cIdx < curves.size(); ++cIdx)
    {
        curves[cIdx]->loadDataAndUpdate(false);
    }

    if (m_showFormations)
    {
        setFormationFieldsUiReadOnly(false);
    }
    else
    {
        setFormationFieldsUiReadOnly(true);
    }

    if ( m_wellLogTrackPlotWidget )
    {
        this->updateWellPathAttributesCollection();
        this->updateWellPathAttributesOnPlot();
        m_wellLogTrackPlotWidget->updateLegend();

        this->updateAxisScaleEngine();
        this->updateFormationNamesOnPlot();
        this->applyXZoomFromVisibleRange();
    }

    this->updateAxisAndGridTickIntervals();
    m_majorTickInterval.uiCapability()->setUiHidden(!m_explicitTickIntervals());
    m_minorTickInterval.uiCapability()->setUiHidden(!m_explicitTickIntervals());

    bool emptyRange = std::abs(m_visibleXRangeMax() - m_visibleXRangeMin) < 1.0e-6 * std::max(1.0, std::max(m_visibleXRangeMax(), m_visibleXRangeMin()));
    m_explicitTickIntervals.uiCapability()->setUiReadOnly(emptyRange);
    m_xAxisGridVisibility.uiCapability()->setUiReadOnly(emptyRange);

    updateAllLegendItems();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setAndUpdateWellPathFormationNamesData(RimCase* rimCase, RimWellPath* wellPath)
{
    m_formationCase = rimCase;
    m_formationTrajectoryType = RimWellLogTrack::WELL_PATH;
    m_formationWellPathForSourceCase = wellPath;
    m_formationSimWellName = "";
    m_formationBranchIndex = -1;

    updateConnectedEditors();
    
    if (m_showFormations)
    {
        updateFormationNamesOnPlot();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setAndUpdateSimWellFormationNamesAndBranchData(RimCase* rimCase, const QString& simWellName, int branchIndex, bool useBranchDetection)
{
    m_formationBranchIndex = branchIndex;
    m_formationBranchDetection = useBranchDetection;

    setAndUpdateSimWellFormationNamesData(rimCase, simWellName);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setAndUpdateSimWellFormationNamesData(RimCase* rimCase, const QString& simWellName)
{
    m_formationCase = rimCase;
    m_formationTrajectoryType = RimWellLogTrack::SIMULATION_WELL;
    m_formationWellPathForSourceCase = nullptr;
    m_formationSimWellName = simWellName;

    updateConnectedEditors();

    if (m_showFormations)
    {
        updateFormationNamesOnPlot();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setAutoScaleXEnabled(bool enabled)
{
    m_isAutoScaleXEnabled = enabled;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setXAxisTitle(const QString& text)
{
    m_xAxisTitle = text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogTrack::depthPlotTitle() const
{
    RimWellLogPlot* parent;
    this->firstAncestorOrThisOfTypeAsserted(parent);

    return parent->depthPlotTitle();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimWellLogTrack::widthScaleFactor() const
{
    return static_cast<int>(m_widthScaleFactor());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setWidthScaleFactor(WidthScaleFactor scaleFactor)
{
    m_widthScaleFactor = scaleFactor;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setFormationWellPath(RimWellPath* wellPath)
{
    m_formationWellPathForSourceCase = wellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath* RimWellLogTrack::formationWellPath() const
{
    return m_formationWellPathForSourceCase;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setFormationSimWellName(const QString& simWellName)
{
    m_formationSimWellName = simWellName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setFormationBranchIndex(int branchIndex)
{
    m_formationBranchIndex = branchIndex;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setFormationCase(RimCase* rimCase)
{
    m_formationCase = rimCase;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setFormationTrajectoryType(TrajectoryType trajectoryType)
{
    m_formationTrajectoryType = trajectoryType;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCase* RimWellLogTrack::formationNamesCase() const
{
    return m_formationCase();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::recreateViewer()
{
    if (m_wellLogTrackPlotWidget == nullptr)
    {
        m_wellLogTrackPlotWidget = new RiuWellLogTrack(this);
        updateAxisScaleEngine();

        for (size_t cIdx = 0; cIdx < curves.size(); ++cIdx)
        {
            curves[cIdx]->setParentQwtPlotNoReplot(this->m_wellLogTrackPlotWidget);
        }

        this->m_wellLogTrackPlotWidget->replot();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::detachAllCurves()
{
    for (RimPlotCurve* curve : curves)
    {
        curve->detachQwtCurve();
    }
    for (RimPlotCurve* curve : m_wellPathAttributeCurves)
    {
        curve->detachQwtCurve();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::reattachAllCurves()
{
    for (RimPlotCurve* curve : curves)
    {
        curve->reattachQwtCurve();
    }
    for (RimPlotCurve* curve : m_wellPathAttributeCurves)
    {
        curve->reattachQwtCurve();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::updateParentPlotZoom()
{
    if (m_wellLogTrackPlotWidget)
    {
        RimWellLogPlot* wellLogPlot;
        firstAncestorOrThisOfType(wellLogPlot);
        if (wellLogPlot)
        {
           wellLogPlot->updateDepthZoom();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::calculateXZoomRangeAndUpdateQwt()
{
    this->calculateXZoomRange();
    this->applyXZoomFromVisibleRange();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::applyXZoomFromVisibleRange()
{
    if (!m_wellLogTrackPlotWidget) return;

    m_wellLogTrackPlotWidget->setXRange(m_visibleXRangeMin, m_visibleXRangeMax);

    // Attribute range. Double the width of the radius (thus same as diameter) to allow for labels and casing shoe.
    double attributeRangeMax = RimWellPathAttribute::MAX_DIAMETER_IN_INCHES;
    double attributeRangeMin = 0.0;
    if (m_showWellPathAttributeBothSides)
    {
        attributeRangeMin = -attributeRangeMax;
    }
    else if (m_wellPathAttributeCollection)
    {
        attributeRangeMax = 0.75 * RimWellPathAttribute::MAX_DIAMETER_IN_INCHES;
        attributeRangeMin = 0.5  * RimWellPathAttribute::MAX_DIAMETER_IN_INCHES;
        for (const RimWellPathAttribute* attribute : m_wellPathAttributeCollection->attributes())
        {
 	       attributeRangeMin = std::min(attributeRangeMin, attribute->diameterInInches() * 0.5);
        }
    }
    m_wellLogTrackPlotWidget->setXRange(attributeRangeMin, attributeRangeMax, QwtPlot::xBottom);

    m_wellLogTrackPlotWidget->replot();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::calculateXZoomRange()
{
    std::map<int, std::vector<RimWellFlowRateCurve*>> stackCurveGroups = visibleStackedCurves();
    for (const std::pair<int, std::vector<RimWellFlowRateCurve*>>& curveGroup : stackCurveGroups)
    {
        for (RimWellFlowRateCurve* stCurve : curveGroup.second) stCurve->updateStackedPlotData();
    }

    if (!m_isAutoScaleXEnabled())
    {
        return;
    }

    double minValue = HUGE_VAL;
    double maxValue = -HUGE_VAL;

    size_t visibleCurves = 0u;
    for (auto curve : curves)
    {
        double minCurveValue = HUGE_VAL;
        double maxCurveValue = -HUGE_VAL;

        if (curve->isCurveVisible())
        {
            visibleCurves++;
            if (curve->xValueRange(&minCurveValue, &maxCurveValue))
            {
                if (minCurveValue < minValue)
                {
                    minValue = minCurveValue;
                }

                if (maxCurveValue > maxValue)
                {
                    maxValue = maxCurveValue;
                }
            }
        }
    }

    if (minValue == HUGE_VAL)
    {
        if (visibleCurves)
        {
            minValue = RI_LOGPLOTTRACK_MINX_DEFAULT;
            maxValue = RI_LOGPLOTTRACK_MAXX_DEFAULT;
        }
        else
        {
            // Empty axis when there are no curves
            minValue = 0;
            maxValue = 0;
        }
    }

    if (m_minorTickInterval() != 0.0)
    {
        std::tie(minValue, maxValue) = adjustXRange(minValue, maxValue, m_minorTickInterval());
    }

    m_visibleXRangeMin = minValue;
    m_visibleXRangeMax = maxValue;

    computeAndSetXRangeMinForLogarithmicScale();
    updateEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::updateEditors()
{
    this->updateConnectedEditors();

    RimWellLogPlot* plot = nullptr;
    firstAncestorOrThisOfTypeAsserted(plot);
    plot->updateConnectedEditors();

    RimWellRftPlot* rftPlot(nullptr);

    firstAncestorOrThisOfType(rftPlot);

    if (rftPlot)
    {
        rftPlot->updateConnectedEditors();
    }
    else
    {
        RimWellPltPlot* pltPlot(nullptr);
        firstAncestorOrThisOfType(pltPlot);

        if (pltPlot)
        {
            pltPlot->updateConnectedEditors();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setVisibleXRange(double minValue, double maxValue)
{
    this->setAutoScaleXEnabled(false);
    m_visibleXRangeMin = minValue;
    m_visibleXRangeMax = maxValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setTickIntervals(double majorTickInterval, double minorTickInterval)
{
    m_explicitTickIntervals = true;
    m_majorTickInterval = majorTickInterval;
    m_minorTickInterval = minorTickInterval;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setXAxisGridVisibility(RimWellLogPlot::AxisGridVisibility gridLines)
{
    m_xAxisGridVisibility = gridLines;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setShowFormations(bool on)
{
    m_showFormations = on;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogTrack::showFormations() const
{
    return m_showFormations;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setShowFormationLabels(bool on)
{
    m_showFormationLabels = on;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setShowWellPathAttributes(bool on)
{
    m_showWellPathAttributes = on;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogTrack::showWellPathAttributes() const
{
    return m_showWellPathAttributes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setWellPathAttributesSource(RimWellPath* wellPath)
{
    m_wellPathAttributeSource = wellPath;
    updateWellPathAttributesCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath* RimWellLogTrack::wellPathAttributeSource() const
{
    return m_wellPathAttributeSource;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogCurve* RimWellLogTrack::curveDefinitionFromCurve(const QwtPlotCurve* curve) const
{
    for (size_t idx = 0; idx < curves.size(); idx++)
    {
        if (curves[idx]->qwtPlotCurve() == curve)
        {
            return curves[idx];
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_userName);
    caf::PdmUiGroup* formationGroup = uiOrdering.addNewGroup("Zonation/Formation Names");
    
    formationGroup->add(&m_showFormations);
    formationGroup->add(&m_showFormationLabels);

    if (!m_formationsForCaseWithSimWellOnly)
    {
        formationGroup->add(&m_formationSource);
    }
    else
    {
        m_formationSource = CASE;
    }

    if (m_formationSource() == CASE)
    {
        formationGroup->add(&m_formationCase);

        if (!m_formationsForCaseWithSimWellOnly)
        {
            formationGroup->add(&m_formationTrajectoryType);

            if (m_formationTrajectoryType() == WELL_PATH)
            {
                formationGroup->add(&m_formationWellPathForSourceCase);
            }
        }

        if (m_formationsForCaseWithSimWellOnly || m_formationTrajectoryType() == SIMULATION_WELL)
        {
            formationGroup->add(&m_formationSimWellName);

            RiaSimWellBranchTools::appendSimWellBranchFieldsIfRequiredFromSimWellName(formationGroup,
                                                                                      m_formationSimWellName,
                                                                                      m_formationBranchDetection,
                                                                                      m_formationBranchIndex);
        }
    }
    else if (m_formationSource() == WELL_PICK_FILTER)
    {
        formationGroup->add(&m_formationWellPathForSourceWellPath);
        if (m_formationWellPathForSourceWellPath())
        {
            formationGroup->add(&m_formationLevel);
            formationGroup->add(&m_showformationFluids);
        }
    }

    caf::PdmUiGroup* attributeGroup = uiOrdering.addNewGroup("Well Attributes");
    attributeGroup->add(&m_showWellPathAttributes);
    attributeGroup->add(&m_showWellPathAttributeBothSides);
    attributeGroup->add(&m_wellPathAttributesInLegend);
    attributeGroup->add(&m_wellPathAttributeSource);

    uiOrderingForXAxisSettings(uiOrdering);

    caf::PdmUiGroup* trackSettingsGroup = uiOrdering.addNewGroup("Track Settings");
    trackSettingsGroup->add(&m_widthScaleFactor);

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RimWellLogTrack::curveIndex(RimWellLogCurve* curve)
{
    return curves.index(curve);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellLogTrack::isVisible()
{
    return m_show;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::updateAxisScaleEngine()
{
    if (m_isLogarithmicScaleEnabled)
    {
        m_wellLogTrackPlotWidget->setAxisScaleEngine(QwtPlot::xTop, new QwtLogScaleEngine);
        
        // NB! Must assign scale engine to bottom in order to make QwtPlotGrid work
        m_wellLogTrackPlotWidget->setAxisScaleEngine(QwtPlot::xBottom, new QwtLogScaleEngine);
    }
    else
    {
        m_wellLogTrackPlotWidget->setAxisScaleEngine(QwtPlot::xTop, new RiuQwtLinearScaleEngine);

        // NB! Must assign scale engine to bottom in order to make QwtPlotGrid work
        m_wellLogTrackPlotWidget->setAxisScaleEngine(QwtPlot::xBottom, new RiuQwtLinearScaleEngine);
    }
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogTrack::isFirstVisibleTrackInPlot() const
{
    RimWellLogPlot* plot = nullptr;
    firstAncestorOrThisOfTypeAsserted(plot);
    size_t ownIndex = plot->trackIndex(this);
    return plot->firstVisibleTrackIndex() == ownIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RimWellLogTrack::adjustXRange(double minValue, double maxValue, double tickInterval)
{
    double minRemainder = std::fmod(minValue, tickInterval);
    double maxRemainder = std::fmod(maxValue, tickInterval);
    double adjustedMin = minValue - minRemainder;
    double adjustedMax = maxValue + (tickInterval - maxRemainder);
    return std::make_pair(adjustedMin, adjustedMax);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::updateWellPathAttributesCollection()
{
    m_wellPathAttributeCollection = nullptr;
    if (m_wellPathAttributeSource)
    {
        std::vector<RimWellPathAttributeCollection*> attributeCollection;
        m_wellPathAttributeSource->descendantsIncludingThisOfType(attributeCollection);
        if (!attributeCollection.empty())
        {
            m_wellPathAttributeCollection = attributeCollection.front();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::computeAndSetXRangeMinForLogarithmicScale()
{
    if (m_isAutoScaleXEnabled && m_isLogarithmicScaleEnabled)
    {
        double pos = HUGE_VAL;
        double neg = -HUGE_VAL;

        for (size_t cIdx = 0; cIdx < curves.size(); cIdx++)
        {
            if (curves[cIdx]->isCurveVisible() && curves[cIdx]->curveData())
            {
                RigStatisticsCalculator::posNegClosestToZero(curves[cIdx]->curveData()->xPlotValues(), pos, neg);
            }
        }

        if (pos != HUGE_VAL)
        {
            m_visibleXRangeMin = pos;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setLogarithmicScale(bool enable)
{
    m_isLogarithmicScaleEnabled = enable;

    updateAxisScaleEngine();
    computeAndSetXRangeMinForLogarithmicScale();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::map<int, std::vector<RimWellFlowRateCurve*>> RimWellLogTrack::visibleStackedCurves()
{
    std::map<int, std::vector<RimWellFlowRateCurve*>> stackedCurves;
    for (RimWellLogCurve* curve: curves)
    {
        if (curve && curve->isCurveVisible() )
        {
            RimWellFlowRateCurve* wfrCurve = dynamic_cast<RimWellFlowRateCurve*>(curve);
            if (wfrCurve != nullptr)
            {
                stackedCurves[wfrCurve->groupId()].push_back(wfrCurve);
            }
        }
    }

    return stackedCurves;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellLogTrack::description()
{
    return m_userName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimWellLogCurve* > RimWellLogTrack::curvesVector()
{
    std::vector<RimWellLogCurve* > curvesVector;

    for (RimWellLogCurve* curve : curves)
    {
        curvesVector.push_back(curve);
    }

    return curvesVector;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::uiOrderingForRftPltFormations(caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* formationGroup = uiOrdering.addNewGroup("Zonation/Formation Names");
    formationGroup->setCollapsedByDefault(true);
    formationGroup->add(&m_showFormations);
    formationGroup->add(&m_formationSource);
    if (m_formationSource == CASE)
    {
        formationGroup->add(&m_formationCase);
    }
    if (m_formationSource == WELL_PICK_FILTER)
    {
        if (m_formationWellPathForSourceWellPath() && m_formationWellPathForSourceWellPath()->hasFormations())
        {
            formationGroup->add(&m_formationLevel);
            formationGroup->add(&m_showformationFluids);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::uiOrderingForXAxisSettings(caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* gridGroup = uiOrdering.addNewGroup("X Axis Settings");
    gridGroup->add(&m_isLogarithmicScaleEnabled);
    gridGroup->add(&m_visibleXRangeMin);
    gridGroup->add(&m_visibleXRangeMax);
    gridGroup->add(&m_xAxisGridVisibility);
    gridGroup->add(&m_explicitTickIntervals);
    gridGroup->add(&m_majorTickInterval);
    gridGroup->add(&m_minorTickInterval);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setFormationsForCaseWithSimWellOnly(bool caseWithSimWellOnly)
{
    m_formationsForCaseWithSimWellOnly = caseWithSimWellOnly;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigEclipseWellLogExtractor* RimWellLogTrack::createSimWellExtractor(RimWellLogPlotCollection* wellLogCollection, RimCase* rimCase, const QString& simWellName, int branchIndex, bool useBranchDetection)
{
    if (!wellLogCollection) return nullptr;

    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(rimCase);
    if (!eclipseCase) return nullptr;
    
    std::vector<const RigWellPath*> wellPaths = RiaSimWellBranchTools::simulationWellBranches(simWellName, useBranchDetection);
    
    if (wellPaths.size() == 0) return nullptr;
    
    CVF_ASSERT(branchIndex < static_cast<int>(wellPaths.size()));
     
    return (wellLogCollection->findOrCreateSimWellExtractor(simWellName,
                                                            QString("Find or create sim well extractor"), 
                                                            wellPaths[branchIndex], 
                                                            eclipseCase->eclipseCaseData()));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CurveSamplingPointData RimWellLogTrack::curveSamplingPointData(RigEclipseWellLogExtractor* extractor, RigResultAccessor* resultAccessor)
{
    CurveSamplingPointData curveData;

    curveData.md = extractor->cellIntersectionMDs();
    curveData.tvd = extractor->cellIntersectionTVDs();
    
    extractor->curveData(resultAccessor, &curveData.data);
    
    return curveData;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CurveSamplingPointData RimWellLogTrack::curveSamplingPointData(RigGeoMechWellLogExtractor* extractor, const RigFemResultAddress& resultAddress)
{
    CurveSamplingPointData curveData;

    curveData.md = extractor->cellIntersectionMDs();
    curveData.tvd = extractor->cellIntersectionTVDs();

    extractor->curveData(resultAddress, 0, &curveData.data);
    return curveData;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimWellLogTrack::formationNameIndexToName(RimCase* rimCase, const std::vector<int>& formationNameInidces)
{
    std::vector<QString> availableFormationNames = RimWellLogTrack::formationNamesVector(rimCase);

    std::vector<QString> formationNames;

    for (int index : formationNameInidces)
    {
        formationNames.push_back(availableFormationNames[index]);
    }

    return formationNames;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::findFormationNamesToPlot(const CurveSamplingPointData&           curveData,
                                               const std::vector<QString>&             formationNamesVector,
                                               RimWellLogPlot::DepthTypeEnum           depthType,
                                               std::vector<QString>*                   formationNamesToPlot,
                                               std::vector<std::pair<double, double>>* yValues)
{
    if (formationNamesVector.empty()) return;

    std::vector<size_t> formationNameIndicesFromCurve;

    for (double nameIdx : curveData.data)
    {
        if (nameIdx != std::numeric_limits<double>::infinity())
        {
            formationNameIndicesFromCurve.push_back(static_cast<size_t>(round(nameIdx)));
        }
        else
        {
            formationNameIndicesFromCurve.push_back(std::numeric_limits<size_t>::max());
        }
    }

    if (formationNameIndicesFromCurve.empty()) return;

    std::vector<double> depthVector;

    if (depthType == RimWellLogPlot::MEASURED_DEPTH || depthType == RimWellLogPlot::PSEUDO_LENGTH)
    {
        depthVector = curveData.md;
    }
    else if(depthType == RimWellLogPlot::TRUE_VERTICAL_DEPTH)
    {
        depthVector = curveData.tvd;
    }

    if (depthVector.empty()) return;

    double currentYStart = depthVector[0];
    size_t prevNameIndex = formationNameIndicesFromCurve[0];
    size_t currentNameIndex;

    for (size_t i = 1; i < formationNameIndicesFromCurve.size(); i++)
    {
        currentNameIndex = formationNameIndicesFromCurve[i];
        if (currentNameIndex != std::numeric_limits<size_t>::max() && currentNameIndex != prevNameIndex)
        {
            if (prevNameIndex < formationNamesVector.size())
            {
                formationNamesToPlot->push_back(formationNamesVector[prevNameIndex]);
                yValues->push_back(std::make_pair(currentYStart, depthVector[i - 1]));
            }

            currentYStart = depthVector[i];
            prevNameIndex = currentNameIndex;
        }
    }

    size_t lastFormationIdx = formationNameIndicesFromCurve.back();
    if (lastFormationIdx < formationNamesVector.size())
    {
        formationNamesToPlot->push_back(formationNamesVector[lastFormationIdx]);
        yValues->push_back(std::make_pair(currentYStart, depthVector.back()));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimWellLogTrack::formationNamesVector(RimCase* rimCase)
{
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(rimCase);
    RimGeoMechCase* geoMechCase = dynamic_cast<RimGeoMechCase*>(rimCase);

    if (eclipseCase)
    {
        if (eclipseCase->eclipseCaseData()->activeFormationNames())
        {
            return eclipseCase->eclipseCaseData()->activeFormationNames()->formationNames();
        }
    }
    else if (geoMechCase)
    {
        if (geoMechCase->geoMechData()->femPartResults()->activeFormationNames())
        {
            return geoMechCase->geoMechData()->femPartResults()->activeFormationNames()->formationNames();
        }
    }

    return std::vector<QString>();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setFormationFieldsUiReadOnly(bool readOnly /*= true*/)
{
    m_showFormationLabels.uiCapability()->setUiReadOnly(readOnly);
    m_formationSource.uiCapability()->setUiReadOnly(readOnly);
    m_formationTrajectoryType.uiCapability()->setUiReadOnly(readOnly);
    m_formationSimWellName.uiCapability()->setUiReadOnly(readOnly);
    m_formationCase.uiCapability()->setUiReadOnly(readOnly);
    m_formationWellPathForSourceCase.uiCapability()->setUiReadOnly(readOnly);
    m_formationWellPathForSourceWellPath.uiCapability()->setUiReadOnly(readOnly);
    m_formationBranchDetection.uiCapability()->setUiReadOnly(readOnly);
    m_formationBranchIndex.uiCapability()->setUiReadOnly(readOnly);
    m_formationLevel.uiCapability()->setUiReadOnly(readOnly);
    m_showformationFluids.uiCapability()->setUiReadOnly(readOnly);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::updateFormationNamesOnPlot()
{
    removeFormationNames();

    if (m_showFormations == false) return;

    if (m_annotationTool == nullptr)
    {
        m_annotationTool = std::unique_ptr<RiuPlotAnnotationTool>(new RiuPlotAnnotationTool());
    }

    std::vector<QString> formationNamesToPlot;
    std::vector<std::pair<double, double>> yValues;

    RimWellLogPlot* plot = nullptr;
    firstAncestorOrThisOfTypeAsserted(plot);

    if (m_formationSource == CASE)
    {
        if ((m_formationSimWellName == QString("None") && m_formationWellPathForSourceCase == nullptr) || m_formationCase == nullptr) return;

        RimMainPlotCollection* mainPlotCollection;
        this->firstAncestorOrThisOfTypeAsserted(mainPlotCollection);

        RimWellLogPlotCollection* wellLogCollection = mainPlotCollection->wellLogPlotCollection();

        CurveSamplingPointData curveData;

        RigEclipseWellLogExtractor* eclWellLogExtractor = nullptr;
        RigGeoMechWellLogExtractor* geoMechWellLogExtractor = nullptr;

        if (m_formationTrajectoryType == SIMULATION_WELL)
        {
            eclWellLogExtractor = RimWellLogTrack::createSimWellExtractor(wellLogCollection, 
                                                                          m_formationCase, 
                                                                          m_formationSimWellName, 
                                                                          m_formationBranchIndex,
                                                                          m_formationBranchDetection);
        }
        else
        {
            eclWellLogExtractor = RiaExtractionTools::wellLogExtractorEclipseCase(m_formationWellPathForSourceCase,
                                                                                  dynamic_cast<RimEclipseCase*>(m_formationCase()));
        }

        if (eclWellLogExtractor)
        {
            RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_formationCase());
            cvf::ref<RigResultAccessor> resultAccessor = RigResultAccessorFactory::createFromNameAndType(eclipseCase->eclipseCaseData(),
                                                                                                         0,
                                                                                                         RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                                                                         0,
                                                                                                         RiaDefines::activeFormationNamesResultName(),
                                                                                                         RiaDefines::FORMATION_NAMES);

            curveData = RimWellLogTrack::curveSamplingPointData(eclWellLogExtractor, resultAccessor.p());
        }
        else
        {
            geoMechWellLogExtractor = RiaExtractionTools::wellLogExtractorGeoMechCase(m_formationWellPathForSourceCase,
                                                                                  dynamic_cast<RimGeoMechCase*>(m_formationCase()));
            if (!geoMechWellLogExtractor) return;

            std::string activeFormationNamesResultName = RiaDefines::activeFormationNamesResultName().toStdString();
            curveData = RimWellLogTrack::curveSamplingPointData(geoMechWellLogExtractor,
                                                                RigFemResultAddress(RIG_FORMATION_NAMES, activeFormationNamesResultName, ""));
        }

        std::vector<QString> formationNamesVector = RimWellLogTrack::formationNamesVector(m_formationCase);

        RimWellLogTrack::findFormationNamesToPlot(curveData,
                                                  formationNamesVector,
                                                  plot->depthType(),
                                                  &formationNamesToPlot,
                                                  &yValues);
        
        m_annotationTool->attachFormationNames(this->viewer(), formationNamesToPlot, yValues, m_showFormationLabels());
    }
    else if (m_formationSource() == WELL_PICK_FILTER)
    {
        if (m_formationWellPathForSourceWellPath == nullptr) return;

        if (!(plot->depthType() == RimWellLogPlot::MEASURED_DEPTH || plot->depthType() == RimWellLogPlot::TRUE_VERTICAL_DEPTH))
        {
            return;
        }

        std::vector<double> yValues;

        const RigWellPathFormations* formations = m_formationWellPathForSourceWellPath->formationsGeometry();
        if (!formations) return;


        formations->depthAndFormationNamesUpToLevel(m_formationLevel(), &formationNamesToPlot, &yValues, m_showformationFluids(), plot->depthType());
        
        m_annotationTool->attachWellPicks(this->viewer(), formationNamesToPlot, yValues);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::updateWellPathAttributesOnPlot()
{
    for (RimWellPathAttributeCurve* curve : m_wellPathAttributeCurves)
    {
        curve->detachQwtCurve();
    }
    m_wellPathAttributeCurves.deleteAllChildObjects();

    if (m_showWellPathAttributes)
    {
        if (m_wellPathAttributeCollection)
        {
            int index = 0;
            for (RimWellPathAttribute* attribute : m_wellPathAttributeCollection->attributes())
            {                
                cvf::Color3f curveColor = RiaColorTables::wellLogPlotPaletteColors().cycledColor3f(index++);
                {
                    RimWellPathAttributeCurve* positiveCurve = new RimWellPathAttributeCurve(
                        attribute, RimWellPathAttributeCurve::PositiveSide, RimWellPathAttributeCurve::LineCurve);
                    RimWellPathAttributeCurve* negativeCurve = new RimWellPathAttributeCurve(
                        attribute, RimWellPathAttributeCurve::NegativeSide, RimWellPathAttributeCurve::LineCurve);
                    positiveCurve->setColor(curveColor);
                    negativeCurve->setColor(curveColor);
                    positiveCurve->showLegend(m_wellPathAttributesInLegend());
                    negativeCurve->showLegend(false);
                    m_wellPathAttributeCurves.push_back(positiveCurve);
                    m_wellPathAttributeCurves.push_back(negativeCurve);
                }

                if (attribute->type() == RimWellPathAttribute::AttributeCasing)
                {
                    RimWellPathAttributeCurve* positiveSymbol = new RimWellPathAttributeCurve(
                        attribute, RimWellPathAttributeCurve::PositiveSide, RimWellPathAttributeCurve::MarkerSymbol);
                    RimWellPathAttributeCurve* negativeSymbol = new RimWellPathAttributeCurve(
                        attribute, RimWellPathAttributeCurve::NegativeSide, RimWellPathAttributeCurve::MarkerSymbol);
                    positiveSymbol->setColor(curveColor);
                    negativeSymbol->setColor(curveColor);
                    positiveSymbol->showLegend(false);
                    negativeSymbol->showLegend(false);
                    m_wellPathAttributeCurves.push_back(positiveSymbol);
                    m_wellPathAttributeCurves.push_back(negativeSymbol);
                }
            }
        }
        for (RimWellPathAttributeCurve* curve : m_wellPathAttributeCurves)
        {
            curve->loadDataAndUpdate(false);
            if (m_wellLogTrackPlotWidget)
            {
                curve->setParentQwtPlotNoReplot(m_wellLogTrackPlotWidget);
            }
        }
        applyXZoomFromVisibleRange();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::removeFormationNames()
{
    if (m_annotationTool)
    {
        m_annotationTool->detachAllAnnotations();
    }
}
