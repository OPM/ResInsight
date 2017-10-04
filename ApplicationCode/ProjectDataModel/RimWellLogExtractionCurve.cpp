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

#include "RimWellLogExtractionCurve.h"

#include "RiaApplication.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseWellLogExtractor.h"
#include "RigFemPartResultsCollection.h"
#include "RigGeoMechCaseData.h"
#include "RigGeoMechWellLogExtractor.h"
#include "RigResultAccessorFactory.h"
#include "RigSimulationWellCenterLineCalculator.h"
#include "RigSimulationWellCoordsAndMD.h"
#include "RigSingleWellResultsData.h"
#include "RigWellLogCurveData.h"
#include "RigWellPath.h"

#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechResultDefinition.h"
#include "RimGeoMechView.h"
#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimTools.h"
#include "RimWellLogCurve.h"
#include "RimWellLogPlot.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "RiuLineSegmentQwtPlotCurve.h"
#include "RiuWellLogTrack.h"

#include "cafPdmUiTreeOrdering.h"
#include "cafUtils.h"

#include <cmath>
#include "RimEclipseWell.h"

//==================================================================================================
///  
///  
//==================================================================================================

CAF_PDM_SOURCE_INIT(RimWellLogExtractionCurve, "RimWellLogExtractionCurve");


namespace caf
{
template<>
void AppEnum< RimWellLogExtractionCurve::TrajectoryType >::setUp()
{
    addItem(RimWellLogExtractionCurve::WELL_PATH,       "WELL_PATH",        "Well Path");
    addItem(RimWellLogExtractionCurve::SIMULATION_WELL, "SIMULATION_WELL",  "Simulation Well");
    setDefault(RimWellLogExtractionCurve::WELL_PATH);
}
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogExtractionCurve::RimWellLogExtractionCurve()
{
    CAF_PDM_InitObject("Well Log Curve", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_trajectoryType, "TrajectoryType", "Trajectory", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_wellPath, "CurveWellPath", " ", "", "", "");
    m_wellPath.uiCapability()->setUiTreeChildrenHidden(true);

    CAF_PDM_InitField(&m_simWellName, "SimulationWellName", QString("None"), " ", "", "", "");
    CAF_PDM_InitField(&m_branchIndex,  "Branch",          0, " ", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_case, "CurveCase", "Case", "", "", "");
    m_case.uiCapability()->setUiTreeChildrenHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_eclipseResultDefinition, "CurveEclipseResult", "", "", "", "");
    m_eclipseResultDefinition.uiCapability()->setUiHidden(true);
    m_eclipseResultDefinition.uiCapability()->setUiTreeChildrenHidden(true);
    m_eclipseResultDefinition = new RimEclipseResultDefinition;
    m_eclipseResultDefinition->findField("MResultType")->uiCapability()->setUiName("Result Type");

    CAF_PDM_InitFieldNoDefault(&m_geomResultDefinition, "CurveGeomechResult", "", "", "", "");
    m_geomResultDefinition.uiCapability()->setUiHidden(true);
    m_geomResultDefinition.uiCapability()->setUiTreeChildrenHidden(true);
    m_geomResultDefinition = new RimGeoMechResultDefinition;

    CAF_PDM_InitField(&m_timeStep, "CurveTimeStep", 0,"Time Step", "", "", "");

    // Add some space before name to indicate these belong to the Auto Name field
    CAF_PDM_InitField(&m_addCaseNameToCurveName, "AddCaseNameToCurveName", true, "   Case Name", "", "", "");
    CAF_PDM_InitField(&m_addPropertyToCurveName, "AddPropertyToCurveName", true, "   Property", "", "", "");
    CAF_PDM_InitField(&m_addWellNameToCurveName, "AddWellNameToCurveName", true, "   Well Name", "", "", "");
    CAF_PDM_InitField(&m_addTimestepToCurveName, "AddTimestepToCurveName", false, "   Timestep", "", "", "");
    CAF_PDM_InitField(&m_addDateToCurveName, "AddDateToCurveName", true, "   Date", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogExtractionCurve::~RimWellLogExtractionCurve()
{
    clearGeneratedSimWellPaths();

    delete m_geomResultDefinition;
    delete m_eclipseResultDefinition;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::setWellPath(RimWellPath* wellPath)
{
    m_wellPath = wellPath;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPath* RimWellLogExtractionCurve::wellPath() const
{
    return m_wellPath;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::setFromSimulationWellName(const QString& simWellName, int branchIndex)
{
    m_trajectoryType = SIMULATION_WELL;
    m_simWellName = simWellName;
    m_branchIndex = branchIndex;

    clearGeneratedSimWellPaths();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::setCase(RimCase* rimCase)
{
    m_case = rimCase;
    clearGeneratedSimWellPaths();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCase* RimWellLogExtractionCurve::rimCase() const
{
    return m_case;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::setPropertiesFromView(RimView* view)
{
    m_case = view ? view->ownerCase() : NULL;

    RimGeoMechCase* geomCase = dynamic_cast<RimGeoMechCase*>(m_case.value());
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case.value());
    m_eclipseResultDefinition->setEclipseCase(eclipseCase);
    m_geomResultDefinition->setGeoMechCase(geomCase);

    RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(view);
    if (eclipseView)
    {
        m_eclipseResultDefinition->simpleCopy(eclipseView->cellResult());

        m_timeStep = eclipseView->currentTimeStep();
    }

    RimGeoMechView* geoMechView = dynamic_cast<RimGeoMechView*>(view);
    if (geoMechView)
    {
        m_geomResultDefinition->setResultAddress(geoMechView->cellResultResultDefinition()->resultAddress());
        m_timeStep = geoMechView->currentTimeStep();
    }

    clearGeneratedSimWellPaths();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::clampTimestep()
{
    if (m_case)
    {
        if (m_timeStep > m_case->timeStepStrings().size() - 1)
        {
            m_timeStep = m_case->timeStepStrings().size() - 1;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::clampBranchIndex()
{
    int branchCount = static_cast<int>(m_generatedSimulationWellPathBranches.size());
    if ( branchCount > 0 )
    {
        if      ( m_branchIndex >= branchCount ) m_branchIndex = branchCount - 1;
        else if ( m_branchIndex < 0 )           m_branchIndex = 0;
    }
    else
    {
        m_branchIndex = -1;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimWellLogCurve::fieldChangedByUi(changedField, oldValue, newValue);

    if (changedField == &m_case)
    {
        clampTimestep();
        
        auto wellNameSet = findSortedWellNames();
        if (!wellNameSet.count(m_simWellName())) m_simWellName = "None";

        clearGeneratedSimWellPaths();

        this->loadDataAndUpdate(true);
    }    
    else if (changedField == &m_wellPath)
    {
        this->loadDataAndUpdate(true);
    }
    else if (changedField == &m_simWellName)
    {
        clearGeneratedSimWellPaths();

        this->loadDataAndUpdate(true);
    }
    else if (changedField == &m_trajectoryType ||
             changedField == &m_branchIndex)
    {
        this->loadDataAndUpdate(true);
    }
    else if (changedField == &m_timeStep)
    {
        this->loadDataAndUpdate(true);
    }

    if (changedField == &m_addCaseNameToCurveName ||
        changedField == &m_addPropertyToCurveName ||
        changedField == &m_addWellNameToCurveName ||
        changedField == &m_addTimestepToCurveName ||
        changedField == &m_addDateToCurveName)
    {
        this->uiCapability()->updateConnectedEditors();
        updateCurveNameAndUpdatePlotLegend();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::onLoadDataAndUpdate(bool updateParentPlot)
{
    RimWellLogCurve::updateCurvePresentation();

    if (isCurveVisible())
    {
        // Make sure we have set correct case data into the result definitions.

        RimGeoMechCase* geomCase = dynamic_cast<RimGeoMechCase*>(m_case.value());
        RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case.value());
        m_eclipseResultDefinition->setEclipseCase(eclipseCase);
        m_geomResultDefinition->setGeoMechCase(geomCase);

        updateGeneratedSimulationWellpath();
        clampBranchIndex();

        RimMainPlotCollection* mainPlotCollection;
        this->firstAncestorOrThisOfTypeAsserted(mainPlotCollection);

        RimWellLogPlotCollection* wellLogCollection = mainPlotCollection->wellLogPlotCollection();

        cvf::ref<RigEclipseWellLogExtractor> eclExtractor;
        if ( m_trajectoryType == WELL_PATH )
        {
            eclExtractor = wellLogCollection->findOrCreateExtractor(m_wellPath, eclipseCase);
        }
        else
        {
            if (m_branchIndex >= 0 && m_branchIndex < static_cast<int>(m_generatedSimulationWellPathBranches.size()) )
            {
                eclExtractor = wellLogCollection->findOrCreateSimWellExtractor(m_simWellName,
                                                                               eclipseCase->caseUserDescription(),
                                                                               m_generatedSimulationWellPathBranches[m_branchIndex].p(),
                                                                               eclipseCase->eclipseCaseData());
            }
        }
        cvf::ref<RigGeoMechWellLogExtractor> geomExtractor = wellLogCollection->findOrCreateExtractor(m_wellPath, geomCase);

        std::vector<double> values;
        std::vector<double> measuredDepthValues;
        std::vector<double> tvDepthValues;

        RiaDefines::DepthUnitType depthUnit = RiaDefines::UNIT_METER;

        if (eclExtractor.notNull())
        {
            measuredDepthValues = eclExtractor->measuredDepth();
            tvDepthValues = eclExtractor->trueVerticalDepth();

            m_eclipseResultDefinition->loadResult();

            cvf::ref<RigResultAccessor> resAcc = RigResultAccessorFactory::createFromResultDefinition(eclipseCase->eclipseCaseData(),
                                                                                                      0,
                                                                                                      m_timeStep,
                                                                                                      m_eclipseResultDefinition);

            if (resAcc.notNull())
            {
                eclExtractor->curveData(resAcc.p(), &values);
            }

            RiaEclipseUnitTools::UnitSystem eclipseUnitsType = eclipseCase->eclipseCaseData()->unitsType();
            if (eclipseUnitsType == RiaEclipseUnitTools::UNITS_FIELD)
            {
                // See https://github.com/OPM/ResInsight/issues/538
                
                depthUnit = RiaDefines::UNIT_FEET;
            }
        }
        else if (geomExtractor.notNull()) // geomExtractor
        {

            measuredDepthValues =  geomExtractor->measuredDepth();
            tvDepthValues = geomExtractor->trueVerticalDepth();

            m_geomResultDefinition->loadResult();

            geomExtractor->curveData(m_geomResultDefinition->resultAddress(), m_timeStep, &values);
        }

        m_curveData = new RigWellLogCurveData;
        if (values.size() && measuredDepthValues.size())
        {
            if (!tvDepthValues.size())
            {
                m_curveData->setValuesAndMD(values, measuredDepthValues, depthUnit, true);
            }
            else
            {
                m_curveData->setValuesWithTVD(values, measuredDepthValues, tvDepthValues, depthUnit);
            }
        }

        RiaDefines::DepthUnitType displayUnit = RiaDefines::UNIT_METER;

        RimWellLogPlot* wellLogPlot;
        firstAncestorOrThisOfType(wellLogPlot);
        CVF_ASSERT(wellLogPlot);
        if (!wellLogPlot) return;

        displayUnit = wellLogPlot->depthUnit();

        if(wellLogPlot->depthType() == RimWellLogPlot::TRUE_VERTICAL_DEPTH)
        {
            m_qwtPlotCurve->setSamples(m_curveData->xPlotValues().data(), m_curveData->trueDepthPlotValues(displayUnit).data(), static_cast<int>(m_curveData->xPlotValues().size()));
        }
        else if (wellLogPlot->depthType() == RimWellLogPlot::MEASURED_DEPTH)
        {
            m_qwtPlotCurve->setSamples(m_curveData->xPlotValues().data(), m_curveData->measuredDepthPlotValues(displayUnit).data(), static_cast<int>(m_curveData->xPlotValues().size()));
        }

        m_qwtPlotCurve->setLineSegmentStartStopIndices(m_curveData->polylineStartStopIndices());

        updateZoomInParentPlot();

        setLogScaleFromSelectedResult();

        if (m_parentQwtPlot) m_parentQwtPlot->replot();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::set<QString> RimWellLogExtractionCurve::findSortedWellNames()
{
    std::set<QString> sortedWellNames;
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case.value());

    if ( eclipseCase && eclipseCase->eclipseCaseData() )
    {
        const cvf::Collection<RigSingleWellResultsData>& wellRes = eclipseCase->eclipseCaseData()->wellResults();

        for ( size_t wIdx = 0; wIdx < wellRes.size(); ++wIdx )
        {
            sortedWellNames.insert(wellRes[wIdx]->m_wellName);
        }
    }

    return sortedWellNames;
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::updateGeneratedSimulationWellpath()
{
    if (m_generatedSimulationWellPathBranches.size()) return; // Already created. Nothing to do

    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case.value());

    if (!(!m_simWellName().isEmpty() && m_simWellName() != "None" && eclipseCase  && eclipseCase->eclipseCaseData()))
    {
        return ;
    }

    RigEclipseCaseData* eclCaseData = eclipseCase->eclipseCaseData();
    const RigSingleWellResultsData* wellResults = eclCaseData->findWellResult(m_simWellName());

    if (!wellResults) return;

    std::vector< std::vector <cvf::Vec3d> > pipeBranchesCLCoords;
    std::vector< std::vector <RigWellResultPoint> > pipeBranchesCellIds;

    RigSimulationWellCenterLineCalculator::calculateWellPipeCenterlineFromWellFrame(eclCaseData,
                                                                                    wellResults,
                                                                                    -1,
                                                                                    true,
                                                                                    true,
                                                                                    pipeBranchesCLCoords,
                                                                                    pipeBranchesCellIds);


    for ( size_t brIdx = 0; brIdx < pipeBranchesCLCoords.size(); ++brIdx )
    {
        auto wellMdCalculator = RigSimulationWellCoordsAndMD(pipeBranchesCLCoords[brIdx]); // Todo, branch index

        cvf::ref<RigWellPath> newWellPath = new RigWellPath();
        newWellPath->m_measuredDepths = wellMdCalculator.measuredDepths();
        newWellPath->m_wellPathPoints = wellMdCalculator.wellPathPoints();

        m_generatedSimulationWellPathBranches.push_back(newWellPath.p() );
    }
}

//--------------------------------------------------------------------------------------------------
/// Clean up existing generated well paths 
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::clearGeneratedSimWellPaths()
{
    RimWellLogPlotCollection* wellLogCollection = nullptr;

    // Need to use this approach, and not firstAnchestor because the curve might not be inside the hierarchy when deleted.

    RimProject * proj = RiaApplication::instance()->project();
    if (proj && proj->mainPlotCollection() ) wellLogCollection = proj->mainPlotCollection()->wellLogPlotCollection();
    
    if (!wellLogCollection) return;

    for ( size_t wpIdx = 0; wpIdx < m_generatedSimulationWellPathBranches.size(); ++wpIdx )
    {
        wellLogCollection->removeExtractors(m_generatedSimulationWellPathBranches[wpIdx].p());
    }

    m_generatedSimulationWellPathBranches.clear();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellLogExtractionCurve::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
   QList<caf::PdmOptionItemInfo> options;

   options = RimWellLogCurve::calculateValueOptions(fieldNeedingOptions, useOptionsOnly);
   if (options.size() > 0) return options;

    if (fieldNeedingOptions == &m_wellPath)
    {
        RimTools::wellPathOptionItems(&options);

        options.push_front(caf::PdmOptionItemInfo("None", nullptr));
    }
    else if (fieldNeedingOptions == &m_case)
    {
        RimTools::caseOptionItems(&options);

        options.push_front(caf::PdmOptionItemInfo("None", nullptr));
    }
    else if (fieldNeedingOptions == &m_timeStep)
    {
        QStringList timeStepNames;

        if (m_case)
        {
            timeStepNames = m_case->timeStepStrings();
        }

        for (int i = 0; i < timeStepNames.size(); i++)
        {
            options.push_back(caf::PdmOptionItemInfo(timeStepNames[i], i));
        }
    }
    else if (fieldNeedingOptions == &m_simWellName)
    {
        std::set<QString> sortedWellNames = this->findSortedWellNames();

        QIcon simWellIcon(":/Well.png");
        for ( const QString& wname: sortedWellNames )
        {
            options.push_back(caf::PdmOptionItemInfo(wname, wname, false, simWellIcon));
        }

        if ( options.size() == 0 )
        {
            options.push_front(caf::PdmOptionItemInfo("None", "None"));
        }
    }
    else if (fieldNeedingOptions == &m_branchIndex)
    {
        updateGeneratedSimulationWellpath();

        size_t branchCount = m_generatedSimulationWellPathBranches.size();

        for ( int bIdx = 0; bIdx < static_cast<int>(branchCount); ++bIdx)
        {
            options.push_back(caf::PdmOptionItemInfo("Branch " + QString::number(bIdx + 1), QVariant::fromValue(bIdx) ));
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
void RimWellLogExtractionCurve::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    RimPlotCurve::updateOptionSensitivity();

    caf::PdmUiGroup* curveDataGroup = uiOrdering.addNewGroup("Curve Data");

    curveDataGroup->add(&m_case);
    
    RimGeoMechCase* geomCase = dynamic_cast<RimGeoMechCase*>(m_case.value());
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case.value());

    curveDataGroup->add(&m_trajectoryType);
    if (m_trajectoryType() == WELL_PATH)
    {
        curveDataGroup->add(&m_wellPath);
    }
    else 
    {
        curveDataGroup->add(&m_simWellName);
        if ( m_generatedSimulationWellPathBranches.size() > 1 )
        {
            curveDataGroup->add(&m_branchIndex);
        }
    }

    if (eclipseCase)
    {
        m_eclipseResultDefinition->uiOrdering(uiConfigName, *curveDataGroup);

    }
    else if (geomCase)
    {
        m_geomResultDefinition->uiOrdering(uiConfigName, *curveDataGroup);
  
    }

    if (   (eclipseCase && m_eclipseResultDefinition->hasDynamicResult())
        ||  geomCase)
    {
        curveDataGroup->add(&m_timeStep);
    }

    caf::PdmUiGroup* appearanceGroup = uiOrdering.addNewGroup("Appearance");
    RimPlotCurve::appearanceUiOrdering(*appearanceGroup);

    caf::PdmUiGroup* nameGroup = uiOrdering.addNewGroup("Curve Name");
    nameGroup->setCollapsedByDefault(true);
    nameGroup->add(&m_showLegend);
    RimPlotCurve::curveNameUiOrdering(*nameGroup);

    if (m_isUsingAutoName)
    {
        nameGroup->add(&m_addWellNameToCurveName);
        nameGroup->add(&m_addCaseNameToCurveName);
        nameGroup->add(&m_addPropertyToCurveName);
        nameGroup->add(&m_addDateToCurveName);
        nameGroup->add(&m_addTimestepToCurveName);
    }


    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::initAfterRead()
{
    RimWellLogCurve::initAfterRead();

    RimGeoMechCase* geomCase = dynamic_cast<RimGeoMechCase*>(m_case.value());
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case.value());

    m_eclipseResultDefinition->setEclipseCase(eclipseCase);
    m_geomResultDefinition->setGeoMechCase(geomCase);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    uiTreeOrdering.skipRemainingChildren(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::setLogScaleFromSelectedResult()
{
    QString resVar = m_eclipseResultDefinition->resultVariable();

    if (resVar.toUpper().contains("PERM"))
    {
        RimWellLogTrack* track = NULL;
        this->firstAncestorOrThisOfType(track);
        if (track)
        {
            if (track->curveCount() == 1)
            {
                track->setLogarithmicScale(true);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellLogExtractionCurve::createCurveAutoName()
{
    RimGeoMechCase* geomCase = dynamic_cast<RimGeoMechCase*>(m_case.value());
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case.value());
    QString generatedCurveName;

    if (m_addWellNameToCurveName )
    {
        generatedCurveName += wellName();
        if (m_trajectoryType == SIMULATION_WELL && m_generatedSimulationWellPathBranches.size() > 1)
        {
           generatedCurveName += " Br" + QString::number(m_branchIndex + 1); 
        }
    }

    if (m_addCaseNameToCurveName && m_case())
    {
        if (!generatedCurveName.isEmpty())
        {
            generatedCurveName += ", ";
        }

        generatedCurveName += m_case->caseUserDescription();
    }

    if (m_addPropertyToCurveName)
    {
        if (!generatedCurveName.isEmpty())
        {
            generatedCurveName += ",";
        }

        generatedCurveName += wellLogChannelName();
    }

    if (m_addTimestepToCurveName || m_addDateToCurveName)
    {
        size_t maxTimeStep = 0;
        
        if (eclipseCase)
        {
            if (eclipseCase->eclipseCaseData())
            {
                maxTimeStep = eclipseCase->eclipseCaseData()->results(m_eclipseResultDefinition->porosityModel())->maxTimeStepCount();
            }
        }
        else if (geomCase)
        {
            if (geomCase->geoMechData())
            {
                maxTimeStep = geomCase->geoMechData()->femPartResults()->frameCount();
            }
        }

        if (m_addDateToCurveName)
        {
            QString dateString = wellDate();
            if (!dateString.isEmpty())
            {
                if (!generatedCurveName.isEmpty())
                {
                    generatedCurveName += ", ";
                }

                generatedCurveName += dateString;
            }
        }

        if (m_addTimestepToCurveName)
        {
            if (!generatedCurveName.isEmpty())
            {
                generatedCurveName += ", ";
            }

            generatedCurveName += QString("[%1/%2]").arg(m_timeStep()).arg(maxTimeStep);
        }
    }

    return generatedCurveName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellLogExtractionCurve::wellLogChannelName() const
{
    RimGeoMechCase* geoMechCase = dynamic_cast<RimGeoMechCase*>(m_case.value());
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case.value());

    QString name;
    if (eclipseCase)
    {
        name = caf::Utils::makeValidFileBasename( m_eclipseResultDefinition->resultVariableUiName());
    }
    else if (geoMechCase)
    {
        QString resCompName = m_geomResultDefinition->resultComponentUiName();
        if (resCompName.isEmpty())
        {
            name = m_geomResultDefinition->resultFieldUiName();
        }
        else
        {
            name = m_geomResultDefinition->resultFieldUiName() + "." + resCompName;
        }
    }

    return name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellLogExtractionCurve::wellName() const
{
    if ( m_trajectoryType() == WELL_PATH )
    {
        if ( m_wellPath )
        {
            return m_wellPath->name();
        }
        else
        {
            return QString();
        }
    }
    else
    {
        return m_simWellName;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellLogExtractionCurve::wellDate() const
{
    RimGeoMechCase* geomCase = dynamic_cast<RimGeoMechCase*>(m_case.value());
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case.value());

    QStringList timeStepNames;

    if (eclipseCase)
    {
        if (eclipseCase->eclipseCaseData())
        {
            timeStepNames = eclipseCase->timeStepStrings();
        }
    }
    else if (geomCase)
    {
        if (geomCase->geoMechData())
        {
            timeStepNames = geomCase->timeStepStrings();
        }
    }

    return (m_timeStep < timeStepNames.size()) ? timeStepNames[m_timeStep] : "";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellLogExtractionCurve::isEclipseCurve() const
{
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case.value());
    if (eclipseCase)
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellLogExtractionCurve::caseName() const
{
    if (m_case)
    {
        return m_case->caseUserDescription();
    }

    return QString();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimWellLogExtractionCurve::rkbDiff() const
{
    if (m_wellPath && m_wellPath->wellPathGeometry())
    {
        RigWellPath* geo = m_wellPath->wellPathGeometry();

        if (geo->hasDatumElevation())
        {
            return geo->datumElevation();
        }

        // If measured depth is zero, use the z-value of the well path points
        if (geo->m_wellPathPoints.size() > 0 && geo->m_measuredDepths.size() > 0)
        {
            double epsilon = 1e-3;

            if (cvf::Math::abs(geo->m_measuredDepths[0]) < epsilon)
            {
                double diff = geo->m_measuredDepths[0] - (-geo->m_wellPathPoints[0].z());

                return diff;
            }
        }
    }

    return HUGE_VAL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RimWellLogExtractionCurve::currentTimeStep() const
{
    return m_timeStep();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::setCurrentTimeStep(int timeStep)
{
    m_timeStep = timeStep;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::setEclipseResultDefinition(const RimEclipseResultDefinition* def)
{
    m_eclipseResultDefinition->simpleCopy(def);
}
