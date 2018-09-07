/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016      Statoil ASA
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

#include "RimWellLogCurveCommonDataSource.h"

#include "RimCase.h"
#include "RimDataSourceSteppingTools.h"
#include "RimEclipseCase.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimTools.h"
#include "RimWellLogExtractionCurve.h"
#include "RimWellLogFileCurve.h"
#include "RimWellLogPlot.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "RiaApplication.h"
#include "RiaSimWellBranchTools.h"

#include "cafPdmUiCheckBoxTristateEditor.h"
#include "cafPdmUiComboBoxEditor.h"

CAF_PDM_SOURCE_INIT(RimWellLogCurveCommonDataSource, "ChangeDataSourceFeatureUi");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogCurveCommonDataSource::RimWellLogCurveCommonDataSource()
{
    CAF_PDM_InitObject("Change Data Source", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_case, "CurveCase", "Case", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_trajectoryType, "TrajectoryType", "Trajectory Type", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_wellPath, "CurveWellPath", "Well Name", "", "", "");

    CAF_PDM_InitField(&m_simWellName, "SimulationWellName", QString("None"), "Well Name", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_branchDetection, "BranchDetection", "Branch Detection", "",
        "Compute branches based on how simulation well cells are organized", "");    
    m_branchDetection.v() = caf::Tristate::State::PartiallyTrue;
    m_branchDetection.uiCapability()->setUiEditorTypeName(caf::PdmUiCheckBoxTristateEditor::uiEditorTypeName());
    CAF_PDM_InitField(&m_branchIndex, "Branch", -1, "Branch Index", "", "", "");

    CAF_PDM_InitField(&m_timeStep, "CurveTimeStep", -1, "Time Step", "", "", "");

    m_case = nullptr;
    m_wellPath = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCase* RimWellLogCurveCommonDataSource::caseToApply() const
{
    return m_case;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::setCaseToApply(RimCase* val)
{
    m_case = val;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimWellLogCurveCommonDataSource::trajectoryTypeToApply() const
{
    return m_trajectoryType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::setTrajectoryTypeToApply(int val)
{
    m_trajectoryType = val;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath* RimWellLogCurveCommonDataSource::wellPathToApply() const
{
    return m_wellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::setWellPathToApply(RimWellPath* val)
{
    m_wellPath = val;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimWellLogCurveCommonDataSource::branchIndexToApply() const
{
    return m_branchIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::setBranchIndexToApply(int val)
{
    m_branchIndex = val;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::Tristate RimWellLogCurveCommonDataSource::branchDetectionToApply() const
{
    return m_branchDetection.v();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::setBranchDetectionToApply(caf::Tristate::State val)
{
    m_branchDetection.v() = val;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogCurveCommonDataSource::simWellNameToApply() const
{
    return m_simWellName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::setSimWellNameToApply(const QString& val)
{
    m_simWellName = val;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimWellLogCurveCommonDataSource::timeStepToApply() const
{
    return m_timeStep;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::setTimeStepToApply(int val)
{
    m_timeStep = val;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::resetDefaultOptions()
{
    setCaseToApply(nullptr);
    setTrajectoryTypeToApply(-1);
    setWellPathToApply(nullptr);
    setBranchIndexToApply(-1);
    setBranchDetectionToApply(caf::Tristate::State::PartiallyTrue);
    setSimWellNameToApply(QString(""));
    setTimeStepToApply(-1);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::updateDefaultOptions(const std::vector<RimWellLogCurve*>& curves, const std::vector<RimWellLogTrack*>& tracks)
{
    // Reset all options in the UI
    resetDefaultOptions();

    // Check to see if the parameters are unique
    std::set<RimCase*>     uniqueCases;
    std::set<int>          uniqueTrajectoryTypes;
    std::set<RimWellPath*> uniqueWellPaths;
    std::set<QString>      uniqueWellNames;
    std::set<int>          uniqueTimeSteps;
    std::set<bool>         uniqueBranchDetection;
    std::set<int>          uniqueBranchIndex;
    for (RimWellLogCurve* curve : curves)
    {
        RimWellLogExtractionCurve* extractionCurve = dynamic_cast<RimWellLogExtractionCurve*>(curve);
        RimWellLogFileCurve*       fileCurve       = dynamic_cast<RimWellLogFileCurve*>(curve);
        if (extractionCurve)
        {
            uniqueCases.insert(extractionCurve->rimCase());
            uniqueTrajectoryTypes.insert(static_cast<int>(extractionCurve->trajectoryType()));
            uniqueWellPaths.insert(extractionCurve->wellPath());
            uniqueWellNames.insert(extractionCurve->wellName());
            uniqueTimeSteps.insert(extractionCurve->currentTimeStep());
            uniqueBranchDetection.insert(extractionCurve->branchDetection());
            uniqueBranchIndex.insert(extractionCurve->branchIndex());
        }
        else if (fileCurve)
        {
            uniqueWellPaths.insert(fileCurve->wellPath());
            uniqueWellNames.insert(fileCurve->wellName());
        }
    }
    for (RimWellLogTrack* track : tracks)
    {
        uniqueTrajectoryTypes.insert(static_cast<int>(RimWellLogExtractionCurve::WELL_PATH));
        uniqueWellPaths.insert(track->wellPathAttributeSource());
        uniqueCases.insert(track->formationNamesCase());
        uniqueWellPaths.insert(track->formationWellPath());
    }

    
    if (uniqueCases.size() == 1u)
    {
        setCaseToApply(*uniqueCases.begin());
    }

    if (uniqueTrajectoryTypes.size() == 1u)
    {
        m_trajectoryType = *uniqueTrajectoryTypes.begin();

        if (uniqueWellPaths.size() == 1u)
        {
            setWellPathToApply(*uniqueWellPaths.begin());
        }
        if (uniqueBranchIndex.size() == 1u)
        {
            setBranchIndexToApply(*uniqueBranchIndex.begin());
        }
        if (uniqueBranchDetection.size() == 1u)
        {
            setBranchDetectionToApply(*uniqueBranchDetection.begin() == true ?
                caf::Tristate::State::True : caf::Tristate::State::False);
        }
        if (uniqueWellNames.size() == 1u)
        {
            setSimWellNameToApply(*uniqueWellNames.begin());
        }
    }

    if (uniqueTimeSteps.size() == 1u)
    {
        setTimeStepToApply(*uniqueTimeSteps.begin());
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::updateDefaultOptions()
{
    RimWellLogPlot* parentPlot = nullptr;
    this->firstAncestorOrThisOfType(parentPlot);
    if (parentPlot)
    {
        std::vector<RimWellLogCurve*> curves;
        parentPlot->descendantsIncludingThisOfType(curves);

        std::vector<RimWellLogTrack*> tracks;
        parentPlot->descendantsIncludingThisOfType(tracks);
        
        this->updateDefaultOptions(curves, tracks);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::updateCurvesAndTracks(std::vector<RimWellLogCurve*>& curves, std::vector<RimWellLogTrack*>& tracks)
{
    std::set<RimWellLogPlot*> plots;
    for (RimWellLogCurve* curve : curves)
    {
        RimWellLogFileCurve*       fileCurve = dynamic_cast<RimWellLogFileCurve*>(curve);
        RimWellLogExtractionCurve* extractionCurve = dynamic_cast<RimWellLogExtractionCurve*>(curve);
        if (fileCurve)
        {
            if (wellPathToApply() != nullptr)
            {
                fileCurve->setWellPath(wellPathToApply());
                if (!fileCurve->wellLogChannelName().isEmpty())
                {
                    RimWellLogFile* logFile = wellPathToApply()->firstWellLogFileMatchingChannelName(fileCurve->wellLogChannelName());
                    fileCurve->setWellLogFile(logFile);
                    RimWellLogPlot* parentPlot = nullptr;
                    fileCurve->firstAncestorOrThisOfTypeAsserted(parentPlot);
                    plots.insert(parentPlot);
                }
            }
        }
        else if (extractionCurve)
        {
            bool updatedSomething = false;
            if (caseToApply() != nullptr)
            {
                extractionCurve->setCase(caseToApply());
                updatedSomething = true;
            }

            if (wellPathToApply() != nullptr)
            {
                extractionCurve->setWellPath(wellPathToApply());
                updatedSomething = true;
            }

            if (m_trajectoryType() != -1)
            {
                extractionCurve->setTrajectoryType(static_cast<RimWellLogExtractionCurve::TrajectoryType>(m_trajectoryType()));
                if (m_trajectoryType() == (int)RimWellLogExtractionCurve::SIMULATION_WELL)
                {

                    if (m_branchDetection().isTrue())
                    {
                        extractionCurve->setBranchDetection(true);
                    }
                    else if (m_branchDetection().isFalse())
                    {
                        extractionCurve->setBranchDetection(false);
                    }

                    if (m_branchIndex() != -1)
                    {
                        extractionCurve->setBranchIndex(m_branchIndex());
                    }
                    if (m_simWellName() != QString(""))
                    {
                        extractionCurve->setWellName(m_simWellName());
                    }
                }
                updatedSomething = true;
            }

            if (timeStepToApply() != -1)
            {
                extractionCurve->setCurrentTimeStep(timeStepToApply());
                updatedSomething = true;
            }
            
            if (updatedSomething)
            {
                RimWellLogPlot* parentPlot = nullptr;
                extractionCurve->firstAncestorOrThisOfTypeAsserted(parentPlot);
                plots.insert(parentPlot);
            }
        }
    }

    for (RimWellLogTrack* track : tracks)
    {
        bool updatedSomething = false;
        if (caseToApply() != nullptr)
        {
            track->setFormationCase(caseToApply());
            updatedSomething = true;
        }

        if (wellPathToApply() != nullptr)
        {
            track->setWellPathAttributesSource(wellPathToApply());
            track->setFormationWellPath(wellPathToApply());
            updatedSomething = true;
        }

        if (updatedSomething)
        {
            RimWellLogPlot* parentPlot = nullptr;
            track->firstAncestorOrThisOfTypeAsserted(parentPlot);
            plots.insert(parentPlot);
        }
    }

    for (RimWellLogPlot* plot : plots)
    {
        plot->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::updateCurvesAndTracks()
{
    RimWellLogPlot* parentPlot = nullptr;
    this->firstAncestorOrThisOfType(parentPlot);
    if (parentPlot)
    {
        std::vector<RimWellLogCurve*> curves;
        parentPlot->descendantsIncludingThisOfType(curves);

        std::vector<RimWellLogTrack*> tracks;
        parentPlot->descendantsIncludingThisOfType(tracks);

        this->updateCurvesAndTracks(curves, tracks);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::applyPrevCase()
{
    modifyCurrentIndex(&m_case, -1);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::applyNextCase()
{
    modifyCurrentIndex(&m_case, 1);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::applyPrevWell()
{
    if (m_trajectoryType() == RimWellLogExtractionCurve::WELL_PATH)
    {
        modifyCurrentIndex(&m_wellPath, -1);
    }
    else if (m_trajectoryType() == RimWellLogExtractionCurve::SIMULATION_WELL)
    {
        modifyCurrentIndex(&m_simWellName, -1);
    }    
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::applyNextWell()
{
    if (m_trajectoryType() == RimWellLogExtractionCurve::WELL_PATH)
    {
        modifyCurrentIndex(&m_wellPath, 1);
    }
    else if (m_trajectoryType() == RimWellLogExtractionCurve::SIMULATION_WELL)
    {
        modifyCurrentIndex(&m_simWellName, 1);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::applyPrevTimeStep()
{
    modifyCurrentIndex(&m_timeStep, -1);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::applyNextTimeStep()
{
    modifyCurrentIndex(&m_timeStep, 1);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimWellLogPlot* parentPlot = nullptr;
    this->firstAncestorOrThisOfType(parentPlot);

    if (changedField == &m_branchDetection)
    {
        if (m_branchDetection().isPartiallyTrue())
        {
            // The Tristate is cycled from false -> partially true -> true
            // Partially true is used for "Mixed state" and is not settable by the user so cycle on to true.
            m_branchDetection.v() = caf::Tristate::State::True;
        }
    }

    this->updateCurvesAndTracks();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellLogCurveCommonDataSource::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    this->updateDefaultOptions();

    if (fieldNeedingOptions == &m_case)
    {
        RimTools::caseOptionItems(&options);

        if (caseToApply() == nullptr)
        {
            options.push_front(caf::PdmOptionItemInfo("Mixed Cases", nullptr));
        }
    }
    else if (fieldNeedingOptions == &m_trajectoryType)
    {
        if (m_trajectoryType() == -1)
        {
            options.push_back(caf::PdmOptionItemInfo("Mixed Trajectory Types", -1));
        }
        std::vector<RimWellLogExtractionCurve::TrajectoryType> trajectoryTypes =
            { RimWellLogExtractionCurve::WELL_PATH, RimWellLogExtractionCurve::SIMULATION_WELL };
        for (RimWellLogExtractionCurve::TrajectoryType trajectoryType : trajectoryTypes)
        {
            caf::PdmOptionItemInfo item(caf::AppEnum<RimWellLogExtractionCurve::TrajectoryType>::uiText(trajectoryType),
                                        static_cast<int>(trajectoryType));
            options.push_back(item);
        }
    }
    else if (fieldNeedingOptions == &m_wellPath)
    {
        RimTools::wellPathOptionItems(&options);
        if (wellPathToApply() == nullptr)
        {
            options.push_front(caf::PdmOptionItemInfo("Mixed Well Paths", nullptr));
        }

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

        if (timeStepToApply() == -1)
        {
            options.push_front(caf::PdmOptionItemInfo("Mixed Time Steps", -1));
        }
    }
    else if (fieldNeedingOptions == &m_simWellName)
    {
        RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case());
        if (eclipseCase)
        {
            std::set<QString> sortedWellNames = eclipseCase->sortedSimWellNames();

            QIcon simWellIcon(":/Well.png");
            for (const QString& wname : sortedWellNames)
            {
                options.push_back(caf::PdmOptionItemInfo(wname, wname, false, simWellIcon));
            }

            if (options.size() == 0)
            {
                options.push_front(caf::PdmOptionItemInfo("None", "None"));
            }

            if (m_simWellName == QString(""))
            {
                options.push_front(caf::PdmOptionItemInfo("Mixed Well Names", ""));
            }
        }
    }
    else if (fieldNeedingOptions == &m_branchIndex)
    {
        bool hasCommonBranchDetection = !m_branchDetection().isPartiallyTrue();
        if (hasCommonBranchDetection)
        {
            bool doBranchDetection = m_branchDetection().isTrue();
            auto branches = RiaSimWellBranchTools::simulationWellBranches(m_simWellName, doBranchDetection);

            options = RiaSimWellBranchTools::valueOptionsForBranchIndexField(branches);

            if (m_branchIndex() == -1)
            {
                options.push_front(caf::PdmOptionItemInfo("Mixed Branches", -1));
            }
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    updateDefaultOptions();

    caf::PdmUiGroup* group = uiOrdering.addNewGroup("Data Source");
    group->add(&m_case);

    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case());
    if (eclipseCase)
    {
        group->add(&m_trajectoryType);
        if (trajectoryTypeToApply() == RimWellLogExtractionCurve::WELL_PATH)
        {
            group->add(&m_wellPath);
        }
        else if (trajectoryTypeToApply() == RimWellLogExtractionCurve::SIMULATION_WELL)
        {
            group->add(&m_simWellName);
            if (RiaSimWellBranchTools::simulationWellBranches(m_simWellName(), true).size() > 1)
            {
                group->add(&m_branchDetection);
                bool hasCommonBranchDetection = !m_branchDetection().isPartiallyTrue();
                if (hasCommonBranchDetection)
                {
                    bool doBranchDetection = m_branchDetection().isTrue();
                    if (RiaSimWellBranchTools::simulationWellBranches(m_simWellName(), doBranchDetection).size() > 1)
                    {
                        group->add(&m_branchIndex);
                    }
                }
            }
        }
    }
    else
    {
        group->add(&m_wellPath);
    }
    group->add(&m_timeStep);
    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::defineEditorAttribute(const caf::PdmFieldHandle* field,
                                                            QString                    uiConfigName,
                                                            caf::PdmUiEditorAttribute* attribute)
{
    caf::PdmUiComboBoxEditorAttribute* myAttr = dynamic_cast<caf::PdmUiComboBoxEditorAttribute*>(attribute);
    if (myAttr)
    {
        if (field == &m_case ||
            field == &m_simWellName ||
            field == &m_wellPath ||
            field == &m_timeStep)
        {
            myAttr->showPreviousAndNextButtons = true;
        }

        QString modifierText;

        if (field == &m_case)
        {
            modifierText = ("(Shift+");
        }
        else if (field == &m_wellPath || field == &m_simWellName)
        {
            modifierText = ("(Ctrl+");
        }
        else if (field == &m_timeStep)
        {
            modifierText = ("(");
        }

        if (!modifierText.isEmpty())
        {
            myAttr->nextButtonText = "Next " + modifierText + "PgDown)";
            myAttr->prevButtonText = "Previous " + modifierText + "PgUp)";
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::modifyCurrentIndex(caf::PdmValueField* field, int indexOffset)
{
    bool useOptionsOnly;
    QList<caf::PdmOptionItemInfo> options = calculateValueOptions(field, &useOptionsOnly);
    RimDataSourceSteppingTools::modifyCurrentIndex(field, options, indexOffset);    
}
