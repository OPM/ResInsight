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

#include "cvfAssert.h"

#include "qwt_scale_engine.h"
#include "RiaSimWellBranchTools.h"

#define RI_LOGPLOTTRACK_MINX_DEFAULT    -10.0
#define RI_LOGPLOTTRACK_MAXX_DEFAULT    100.0


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
        addItem(RimWellLogTrack::WELL_PICK, "WELL_PICK", "Well Pick");
        addItem(RimWellLogTrack::WELL_PICK_FILTER, "WELL_PICK_FILTER", "Well Pick Filter");
        setDefault(RimWellLogTrack::CASE);
    }
   
    template<>
    void AppEnum<RigWellPathFormation::FormationLevel>::setUp()
    {
        addItem(RigWellPathFormation::ALL, "ALL", "All");
        addItem(RigWellPathFormation::GROUP, "GROUP", "Group");
        addItem(RigWellPathFormation::LEVEL0, "LEVEL0", "Main");
        addItem(RigWellPathFormation::LEVEL1, "LEVEL1", "Level 1");
        addItem(RigWellPathFormation::LEVEL2, "LEVEL2", "Level 2");
        addItem(RigWellPathFormation::LEVEL3, "LEVEL3", "Level 3");
        setDefault(RigWellPathFormation::ALL);
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

    CAF_PDM_InitField(&m_isLogarithmicScaleEnabled, "LogarithmicScaleX", false, "Logarithmic Scale", "", "", "");

    CAF_PDM_InitField(&m_showFormations, "ShowFormations", false, "Show Formations", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_formationSource, "FormationSource", "Formation Source", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_formationTrajectoryType, "FormationTrajectoryType", "Trajectory", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_formationWellPath, "FormationWellPath", "Well Path", "", "", "");
    m_formationWellPath.uiCapability()->setUiTreeChildrenHidden(true);

    CAF_PDM_InitField(&m_formationSimWellName, "FormationSimulationWellName", QString("None"), "Simulation Well", "", "", "");
    CAF_PDM_InitField(&m_formationBranchIndex, "FormationBranchIndex", 0, " ", "", "", "");
    CAF_PDM_InitField(&m_formationBranchDetection, "FormationBranchDetection", true, "Branch Detection", "", 
                      "Compute branches based on how simulation well cells are organized", "");

    CAF_PDM_InitFieldNoDefault(&m_formationCase, "FormationCase", "Formation Case", "", "", "");
    m_formationCase.uiCapability()->setUiTreeChildrenHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_formationLevel, "FormationLevel", "Formation Level", "", "", "");

    CAF_PDM_InitField(&m_showformationFluids, "ShowFormationFluids", false, "Show Fluids", "", "", "");
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

        RimWellLogPlot* wellLogPlot;
        this->firstAncestorOrThisOfType(wellLogPlot);
        if (wellLogPlot)
        {
            wellLogPlot->calculateAvailableDepthRange();
            wellLogPlot->updateDepthZoom();

            RiuWellLogPlot* wellLogPlotViewer = dynamic_cast<RiuWellLogPlot*>(wellLogPlot->viewWidget());
            if (wellLogPlotViewer)
            {
                wellLogPlotViewer->updateChildrenLayout();
            }
        }
    }
    else if (changedField == &m_visibleXRangeMin || changedField == &m_visibleXRangeMax)
    {
        m_wellLogTrackPlotWidget->setXRange(m_visibleXRangeMin, m_visibleXRangeMax);
        m_wellLogTrackPlotWidget->replot();
        m_isAutoScaleXEnabled = false;
    }
    else if (changedField == &m_isAutoScaleXEnabled)
    {
        if (m_isAutoScaleXEnabled())
        { 
            this->updateXZoom();
            computeAndSetXRangeMinForLogarithmicScale();

            if (m_wellLogTrackPlotWidget) m_wellLogTrackPlotWidget->replot();
        }
    }
    else if (changedField == &m_isLogarithmicScaleEnabled)
    {
        updateAxisScaleEngine();

        this->updateXZoom();
        computeAndSetXRangeMinForLogarithmicScale();

        m_wellLogTrackPlotWidget->setXRange(m_visibleXRangeMin, m_visibleXRangeMax);

        m_wellLogTrackPlotWidget->replot();
    }
    else if (changedField == &m_showFormations || changedField == &m_formationSource)
    {
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
    else if (changedField == &m_formationCase)
    {
        QList<caf::PdmOptionItemInfo> options;
        RimWellLogTrack::simWellOptionItems(&options, m_formationCase);

        if (options.isEmpty())
        {
            m_formationSimWellName = QString("None");
        }

        loadDataAndUpdate();
    }
    else if (changedField == &m_formationWellPath)
    {
        loadDataAndUpdate();
    }
    else if (changedField == &m_formationSimWellName)
    {
        loadDataAndUpdate();
    }
    else if (changedField == &m_formationTrajectoryType)
    {
        loadDataAndUpdate();
    }
    else if (changedField == &m_formationBranchIndex || 
             changedField == &m_formationBranchDetection)
    {
        m_formationBranchIndex = RiaSimWellBranchTools::clampBranchIndex(m_formationSimWellName, m_formationBranchIndex, m_formationBranchDetection);

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
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellLogTrack::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (options.size() > 0) return options;

    if (fieldNeedingOptions == &m_formationWellPath)
    {
        if (m_formationSource == CASE)
        {
            RimTools::wellPathOptionItems(&options);
        }
        else if(m_formationSource == WELL_PICK || m_formationSource == WELL_PICK_FILTER)
        {
            RimTools::wellPathWithFormationsOptionItems(&options);
        }

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

    size_t curveCount = curves.size();

    for (size_t cIdx = 0; cIdx < curveCount; cIdx++)
    {
        double minCurveDepth = HUGE_VAL;
        double maxCurveDepth = -HUGE_VAL;

        if (curves[cIdx]->isCurveVisible() && curves[cIdx]->depthRange(&minCurveDepth, &maxCurveDepth))
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
        m_wellLogTrackPlotWidget->setDepthTitle(wellLogPlot->depthPlotTitle());
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
        m_wellLogTrackPlotWidget->updateLegend();
        this->updateAxisScaleEngine();
        this->updateFormationNamesOnPlot();
        this->updateXZoomAndParentPlotDepthZoom();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setAndUpdateWellPathFormationNamesData(RimCase* rimCase, RimWellPath* wellPath)
{
    m_formationCase = rimCase;
    m_formationTrajectoryType = RimWellLogTrack::WELL_PATH;
    m_formationWellPath = wellPath;
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
    m_formationWellPath = nullptr;
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
void RimWellLogTrack::setXAxisTitle(const QString& text)
{
    m_xAxisTitle = text;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setFormationWellPath(RimWellPath* wellPath)
{
    m_formationWellPath = wellPath;
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
    for (size_t cIdx = 0; cIdx < curves.size(); ++cIdx)
    {
        curves[cIdx]->detachQwtCurve();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::updateXZoomAndParentPlotDepthZoom()
{
    if (m_wellLogTrackPlotWidget)
    {
        RimWellLogPlot* wellLogPlot;
        firstAncestorOrThisOfType(wellLogPlot);
        if (wellLogPlot)
        {
           wellLogPlot->updateDepthZoom();
        }

        updateXZoom();

        m_wellLogTrackPlotWidget->replot();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::updateXZoom()
{
    std::map<int, std::vector<RimWellFlowRateCurve*>> stackCurveGroups = visibleStackedCurves();
    for (const std::pair<int, std::vector<RimWellFlowRateCurve*>>& curveGroup : stackCurveGroups)
    {
        for (RimWellFlowRateCurve* stCurve : curveGroup.second) stCurve->updateStackedPlotData();
    }

    if (!m_isAutoScaleXEnabled())
    {
        m_wellLogTrackPlotWidget->setXRange(m_visibleXRangeMin, m_visibleXRangeMax);
        m_wellLogTrackPlotWidget->replot();
        return;
    }

    double minValue = HUGE_VAL;
    double maxValue = -HUGE_VAL;

    for (size_t cIdx = 0; cIdx < curves.size(); cIdx++)
    {
        double minCurveValue = HUGE_VAL;
        double maxCurveValue = -HUGE_VAL;

        if (curves[cIdx]->isCurveVisible() && curves[cIdx]->valueRange(&minCurveValue, &maxCurveValue))
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

    if (minValue == HUGE_VAL)
    {
        minValue = RI_LOGPLOTTRACK_MINX_DEFAULT;
        maxValue = RI_LOGPLOTTRACK_MAXX_DEFAULT;
    }

    m_visibleXRangeMin = minValue;
    m_visibleXRangeMax = maxValue;

    computeAndSetXRangeMinForLogarithmicScale();

    if (m_wellLogTrackPlotWidget) m_wellLogTrackPlotWidget->setXRange(m_visibleXRangeMin, m_visibleXRangeMax);

    updateConnectedEditors();
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

    formationGroup->add(&m_formationSource);

    if (m_formationSource() == CASE)
    {
        formationGroup->add(&m_formationCase);

        formationGroup->add(&m_formationTrajectoryType);
        if (m_formationTrajectoryType() == WELL_PATH)
        {
            formationGroup->add(&m_formationWellPath);
        }
        else
        {
            formationGroup->add(&m_formationSimWellName);
            
            RiaSimWellBranchTools::appendSimWellBranchFieldsIfRequiredFromSimWellName(formationGroup,
                                                                                      m_formationSimWellName,
                                                                                      m_formationBranchDetection,
                                                                                      m_formationBranchIndex);
        }
    }
    else if (m_formationSource() == WELL_PICK)
    {
        formationGroup->add(&m_formationWellPath);
    }
    else if (m_formationSource() == WELL_PICK_FILTER)
    {
        formationGroup->add(&m_formationWellPath);
        formationGroup->add(&m_formationLevel);
        formationGroup->add(&m_showformationFluids);
    }

    uiOrderingForVisibleXRange(uiOrdering);

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
        m_wellLogTrackPlotWidget->setAxisScaleEngine(QwtPlot::xTop, new QwtLinearScaleEngine);

        // NB! Must assign scale engine to bottom in order to make QwtPlotGrid work
        m_wellLogTrackPlotWidget->setAxisScaleEngine(QwtPlot::xBottom, new QwtLinearScaleEngine);
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
void RimWellLogTrack::uiOrderingForShowFormationNamesAndCase(caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* formationGroup = uiOrdering.addNewGroup("Zonation/Formation Names");
    formationGroup->setCollapsedByDefault(true);
    formationGroup->add(&m_showFormations);
    formationGroup->add(&m_formationSource);
    if (m_formationSource == CASE)
    {
        formationGroup->add(&m_formationCase);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::uiOrderingForVisibleXRange(caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* gridGroup = uiOrdering.addNewGroup("Visible X Axis Range");
    gridGroup->add(&m_isAutoScaleXEnabled);
    gridGroup->add(&m_isLogarithmicScaleEnabled);
    gridGroup->add(&m_visibleXRangeMin);
    gridGroup->add(&m_visibleXRangeMax);
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
RigEclipseWellLogExtractor* RimWellLogTrack::createWellPathExtractor(RimWellLogPlotCollection* wellLogCollection, RimCase* rimCase, RimWellPath* wellPath)
{
    if (!wellLogCollection) return nullptr;

    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(rimCase);
    return (wellLogCollection->findOrCreateExtractor(wellPath, eclipseCase));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigGeoMechWellLogExtractor* RimWellLogTrack::createGeoMechExtractor(RimWellLogPlotCollection* wellLogCollection, RimCase* rimCase, RimWellPath* wellPath)
{
    RimGeoMechCase* geomCase = dynamic_cast<RimGeoMechCase*>(rimCase);
    return (wellLogCollection->findOrCreateExtractor(wellPath, geomCase));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CurveSamplingPointData RimWellLogTrack::curveSamplingPointData(RigEclipseWellLogExtractor* extractor, RigResultAccessor* resultAccessor)
{
    CurveSamplingPointData curveData;

    curveData.md = extractor->measuredDepth();
    curveData.tvd = extractor->trueVerticalDepth();
    
    extractor->curveData(resultAccessor, &curveData.data);
    
    return curveData;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CurveSamplingPointData RimWellLogTrack::curveSamplingPointData(RigGeoMechWellLogExtractor* extractor, const RigFemResultAddress& resultAddress)
{
    CurveSamplingPointData curveData;

    curveData.md = extractor->measuredDepth();
    curveData.tvd = extractor->trueVerticalDepth();

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
        formationNameIndicesFromCurve.push_back(round(nameIdx));
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
    double prevNameIndex = formationNameIndicesFromCurve[0];
    double currentNameIndex;

    for (size_t i = 1; i < formationNameIndicesFromCurve.size(); i++)
    {
        currentNameIndex = formationNameIndicesFromCurve[i];
        if (currentNameIndex != prevNameIndex)
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

    size_t lastIdx = formationNameIndicesFromCurve.size() - 1;

    formationNamesToPlot->push_back(formationNamesVector[formationNameIndicesFromCurve[lastIdx]]);
    yValues->push_back(std::make_pair(currentYStart, depthVector[lastIdx]));
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
    m_formationSource.uiCapability()->setUiReadOnly(readOnly);
    m_formationTrajectoryType.uiCapability()->setUiReadOnly(readOnly);
    m_formationSimWellName.uiCapability()->setUiReadOnly(readOnly);
    m_formationCase.uiCapability()->setUiReadOnly(readOnly);
    m_formationWellPath.uiCapability()->setUiReadOnly(readOnly);
    m_formationBranchIndex.uiCapability()->setUiReadOnly(readOnly);
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
        if ((m_formationSimWellName == QString("None") && m_formationWellPath == nullptr) || m_formationCase == nullptr) return;

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
            eclWellLogExtractor = RimWellLogTrack::createWellPathExtractor(wellLogCollection, 
                                                                           m_formationCase, 
                                                                           m_formationWellPath);
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
            geoMechWellLogExtractor = RimWellLogTrack::createGeoMechExtractor(wellLogCollection, m_formationCase, m_formationWellPath);
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
        
        m_annotationTool->attachFormationNames(this->viewer(), formationNamesToPlot, yValues);
    }
    else if (m_formationSource() == WELL_PICK)
    {
        if (m_formationWellPath == nullptr) return;
        if (plot->depthType() != RimWellLogPlot::MEASURED_DEPTH) return;

        std::vector<double> yValues;

        const RigWellPathFormations* formations = m_formationWellPath->formationsGeometry();
        if (!formations) return;

        formations->measuredDepthAndFormationNamesWithoutDuplicatesOnDepth(&formationNamesToPlot, &yValues);

        m_annotationTool->attachWellPicks(this->viewer(), formationNamesToPlot, yValues);
    }
    else if (m_formationSource() == WELL_PICK_FILTER)
    {
        if (m_formationWellPath == nullptr) return;
        if (plot->depthType() != RimWellLogPlot::MEASURED_DEPTH) return;

        std::vector<double> yValues;

        const RigWellPathFormations* formations = m_formationWellPath->formationsGeometry();
        if (!formations) return;

        formations->measuredDepthAndFormationNamesUpToLevel(m_formationLevel(), &formationNamesToPlot, &yValues, m_showformationFluids());
        
        m_annotationTool->attachWellPicks(this->viewer(), formationNamesToPlot, yValues);
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
