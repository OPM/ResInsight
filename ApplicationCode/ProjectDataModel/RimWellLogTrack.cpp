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

#include "RiuMainWindow.h"
#include "RiuPlotAnnotationTool.h"
#include "RiuWellLogPlot.h"
#include "RiuWellLogTrack.h"

#include "cvfAssert.h"

#include "qwt_scale_engine.h"

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
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogTrack::RimWellLogTrack()
{
    CAF_PDM_InitObject("Track", ":/WellLogTrack16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_userName, "TrackDescription", "Name", "", "", "");
    m_userName.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitField(&m_show, "Show", true, "Show track", "", "", "");
    m_show.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&curves, "Curves", "",  "", "", "");
    curves.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_visibleXRangeMin, "VisibleXRangeMin", RI_LOGPLOTTRACK_MINX_DEFAULT, "Min", "", "", "");
    CAF_PDM_InitField(&m_visibleXRangeMax, "VisibleXRangeMax", RI_LOGPLOTTRACK_MAXX_DEFAULT, "Max", "", "", "");

    CAF_PDM_InitField(&m_isAutoScaleXEnabled, "AutoScaleX", true, "Auto Scale", "", "", "");

    CAF_PDM_InitField(&m_isLogarithmicScaleEnabled, "LogarithmicScaleX", false, "Logarithmic Scale", "", "", "");

    CAF_PDM_InitField(&m_showFormations, "ShowFormations", false, "Show Formations", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_formationTrajectoryType, "FormationTrajectoryType", "Trajectory", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_formationWellPath, "FormationWellPath", " ", "", "", "");
    m_formationWellPath.uiCapability()->setUiTreeChildrenHidden(true);

    CAF_PDM_InitField(&m_formationSimWellName, "FormationSimulationWellName", QString("None"), " ", "", "", "");
    CAF_PDM_InitField(&m_formationBranchIndex, "FormationBranchIndex", 0, " ", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_formationCase, "FormationCase", "Formation Case", "", "", "");
    m_formationCase.uiCapability()->setUiTreeChildrenHidden(true);

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

    clearGeneratedSimWellPaths(&m_generatedSimulationWellPathBranches);
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
/// Clean up existing generated well paths 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::clearGeneratedSimWellPaths(cvf::Collection<RigWellPath>* generatedSimulationWellPathBranches)
{
    RimWellLogPlotCollection* wellLogCollection = nullptr;

    // Need to use this approach, and not firstAnchestor because the curve might not be inside the hierarchy when deleted.

    RimProject * proj = RiaApplication::instance()->project();
    if (proj && proj->mainPlotCollection()) wellLogCollection = proj->mainPlotCollection()->wellLogPlotCollection();

    if (!wellLogCollection) return;

    for (size_t wpIdx = 0; wpIdx < generatedSimulationWellPathBranches->size(); ++wpIdx)
    {
        wellLogCollection->removeExtractors(generatedSimulationWellPathBranches->at(wpIdx));
    }

    generatedSimulationWellPathBranches->clear();
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
    else if (changedField == &m_showFormations)
    {
        loadDataAndUpdate();
    }
    else if (changedField == &m_formationCase)
    {
        QList<caf::PdmOptionItemInfo> options;
        RimWellLogTrack::simWellOptionItems(&options, m_formationCase);

        if (options.isEmpty())
        {
            m_formationSimWellName = QString("None");
        }

        clearGeneratedSimWellPaths(&m_generatedSimulationWellPathBranches);

        loadDataAndUpdate();
    }
    else if (changedField == &m_formationWellPath)
    {
        loadDataAndUpdate();
    }
    else if (changedField == &m_formationSimWellName)
    {
        clearGeneratedSimWellPaths(&m_generatedSimulationWellPathBranches);

        loadDataAndUpdate();
    }
    else if (changedField == &m_formationTrajectoryType)
    {
        clearGeneratedSimWellPaths(&m_generatedSimulationWellPathBranches);

        loadDataAndUpdate();
    }
    else if (changedField == &m_formationBranchIndex)
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
        RimTools::wellPathOptionItems(&options);

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
        updateGeneratedSimulationWellpath(&m_generatedSimulationWellPathBranches, m_formationSimWellName(), m_formationCase);

        size_t branchCount = m_generatedSimulationWellPathBranches.size();

        for (int bIdx = 0; bIdx < static_cast<int>(branchCount); ++bIdx)
        {
            options.push_back(caf::PdmOptionItemInfo("Branch " + QString::number(bIdx + 1), QVariant::fromValue(bIdx)));
        }

        if (options.size() == 0)
        {
            options.push_front(caf::PdmOptionItemInfo("None", -1));
        }
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
void RimWellLogTrack::removeCurve(RimWellLogCurve* curve)
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
    RimWellLogPlot* wellLogPlot;
    firstAncestorOrThisOfType(wellLogPlot);
    if (wellLogPlot && m_wellLogTrackPlotWidget)
    {
        m_wellLogTrackPlotWidget->setDepthTitle(wellLogPlot->depthPlotTitle());
        m_wellLogTrackPlotWidget->setXTitle(m_xAxisTitle);
    }
    for (size_t cIdx = 0; cIdx < curves.size(); ++cIdx)
    {
        curves[cIdx]->loadDataAndUpdate(true);
    }

    if (m_showFormations)
    {
        setFormationFieldsUiReadOnly(false);
    }
    else
    {
        setFormationFieldsUiReadOnly(true);
    }
   
    updateFormationNamesOnPlot();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::updateFormationNamesData(RimCase* rimCase, TrajectoryType trajectoryType, RimWellPath* wellPath, QString simWellName, int branchIndex)
{
    m_formationCase = rimCase;
    m_formationTrajectoryType = trajectoryType;
    m_formationWellPath = wellPath;
    m_formationSimWellName = simWellName;
    m_formationBranchIndex = branchIndex;

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
void RimWellLogTrack::setFormationBranchIndex(int branchIndex)
{
    m_formationBranchIndex = branchIndex;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RimWellLogTrack::formationBranchIndex() const
{
    return m_formationBranchIndex;
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
void RimWellLogTrack::updateGeneratedSimulationWellpath(cvf::Collection<RigWellPath>* generatedSimulationWellPathBranches, const QString& simWellName, RimCase* rimCase)
{
    if (generatedSimulationWellPathBranches->size()) return; // Already created. Nothing to do

    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(rimCase);

    if (!(!simWellName.isEmpty() && simWellName != QString("None") && eclipseCase  && eclipseCase->eclipseCaseData()))
    {
        return;
    }

    RigEclipseCaseData* eclCaseData = eclipseCase->eclipseCaseData();
    const RigSimWellData* simWellData = eclCaseData->findSimWellData(simWellName);

    if (!simWellData) return;

    std::vector< std::vector <cvf::Vec3d> > pipeBranchesCLCoords;
    std::vector< std::vector <RigWellResultPoint> > pipeBranchesCellIds;

    RigSimulationWellCenterLineCalculator::calculateWellPipeCenterlineFromWellFrame(eclCaseData,
                                                                                    simWellData,
                                                                                    -1,
                                                                                    true,
                                                                                    true,
                                                                                    pipeBranchesCLCoords,
                                                                                    pipeBranchesCellIds);

    for (size_t brIdx = 0; brIdx < pipeBranchesCLCoords.size(); ++brIdx)
    {
        auto wellMdCalculator = RigSimulationWellCoordsAndMD(pipeBranchesCLCoords[brIdx]); // Todo, branch index

        cvf::ref<RigWellPath> newWellPath = new RigWellPath();
        newWellPath->m_measuredDepths = wellMdCalculator.measuredDepths();
        newWellPath->m_wellPathPoints = wellMdCalculator.wellPathPoints();

        generatedSimulationWellPathBranches->push_back(newWellPath.p());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_userName);

    caf::PdmUiGroup* formationGroup = uiOrdering.addNewGroup("Formation Names Properties");
    
    formationGroup->add(&m_showFormations);

    formationGroup->add(&m_formationCase);

    formationGroup->add(&m_formationTrajectoryType);
    if (m_formationTrajectoryType() == WELL_PATH)
    {
        formationGroup->add(&m_formationWellPath);
    }
    else
    {
        updateGeneratedSimulationWellpath(&m_generatedSimulationWellPathBranches, m_formationSimWellName(), m_formationCase);

        formationGroup->add(&m_formationSimWellName);
        if (m_generatedSimulationWellPathBranches.size() > 1)
        {
            formationGroup->add(&m_formationBranchIndex);
        }
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
                if (stackedCurves.count(wfrCurve->groupId()) == 0)
                {
                    stackedCurves.insert(std::make_pair(wfrCurve->groupId(), std::vector<RimWellFlowRateCurve*>()));
                }
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
    caf::PdmUiGroup* formationGroup = uiOrdering.addNewGroup("Formation Names");
    formationGroup->setCollapsedByDefault(true);
    formationGroup->add(&m_showFormations);
    formationGroup->add(&m_formationCase);
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
RigEclipseWellLogExtractor* RimWellLogTrack::createSimWellExtractor(RimWellLogPlotCollection* wellLogCollection, RimCase* rimCase, const QString& simWellName, int branchIndex)
{
    if (!wellLogCollection) return nullptr;

    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(rimCase);

    RimProject* proj = RiaApplication::instance()->project();
    std::vector<const RigWellPath*> wellPaths = proj->simulationWellBranches(simWellName);
    if (wellPaths.size() == 0) return nullptr;

    return (wellLogCollection->findOrCreateSimWellExtractor(simWellName, QString("Find or create sim well extractor"), wellPaths[branchIndex], eclipseCase->eclipseCaseData()));
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

    if ((m_formationSimWellName == QString("None") && m_formationWellPath == nullptr) || m_formationCase == nullptr) return;

    if (m_annotationTool == nullptr)
    {
        m_annotationTool = std::unique_ptr<RiuPlotAnnotationTool>(new RiuPlotAnnotationTool());
    }

    RimMainPlotCollection* mainPlotCollection;
    this->firstAncestorOrThisOfTypeAsserted(mainPlotCollection);

    RimWellLogPlotCollection* wellLogCollection = mainPlotCollection->wellLogPlotCollection();
    

    CurveSamplingPointData curveData;

    RigEclipseWellLogExtractor* eclWellLogExtractor;
    RigGeoMechWellLogExtractor* geoMechWellLogExtractor;

    if (m_formationTrajectoryType == SIMULATION_WELL)
    {
        eclWellLogExtractor = RimWellLogTrack::createSimWellExtractor(wellLogCollection, m_formationCase, m_formationSimWellName, m_formationBranchIndex);
    }
    else
    {
        eclWellLogExtractor = RimWellLogTrack::createWellPathExtractor(wellLogCollection, m_formationCase, m_formationWellPath);
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
        curveData = RimWellLogTrack::curveSamplingPointData(geoMechWellLogExtractor, RigFemResultAddress(RIG_FORMATION_NAMES, RiaDefines::activeFormationNamesResultName().toStdString(), ""));
    }

    RimWellLogPlot* plot;
    firstAncestorOrThisOfTypeAsserted(plot);

    std::vector<QString> formationNamesVector = RimWellLogTrack::formationNamesVector(m_formationCase);
    std::vector<QString> formationNamesToPlot;
    std::vector<std::pair<double, double>> yValues;

    RimWellLogTrack::findFormationNamesToPlot(curveData,
                                              formationNamesVector,
                                              plot->depthType(),
                                              &formationNamesToPlot,
                                              &yValues);

    m_annotationTool->attachFormationNames(this->viewer(), formationNamesToPlot, yValues);
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
