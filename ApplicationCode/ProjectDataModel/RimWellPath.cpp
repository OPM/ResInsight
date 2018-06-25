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

#include "RimWellPath.h"

#include "RiaApplication.h"
#include "RiaSimWellBranchTools.h"
#include "RiaWellNameComparer.h"

#include "RifWellPathFormationsImporter.h"
#include "RifWellPathImporter.h"

#include "RigWellPath.h"

#include "Rim3dWellLogCurve.h"
#include "Rim3dWellLogCurveCollection.h"
#include "RimFishbonesMultipleSubs.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimTools.h"
#include "RimWellLogFile.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellPathCollection.h"
#include "RimWellPathCompletions.h"
#include "RimWellPathFracture.h"
#include "RimWellPathFractureCollection.h"

#include "RiuMainWindow.h"

#include "cafPdmUiTreeOrdering.h"
#include "cafUtils.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QString>

#include <regex>

CAF_PDM_SOURCE_INIT(RimWellPath, "WellPathBase");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const char RimWellPath::SIM_WELL_NONE_UI_TEXT[] = "None";

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPath::RimWellPath()
{
    CAF_PDM_InitObject("WellPath", ":/Well.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_name,               "WellPathName",                         "Name", "", "", "");
    m_name.uiCapability()->setUiReadOnly(true);
    m_name.xmlCapability()->setIOWritable(false);
    m_name.xmlCapability()->setIOReadable(false);
    m_name.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_datumElevation, "DatumElevation", "Datum Elevation", "", "", "");
    m_datumElevation.uiCapability()->setUiReadOnly(true);
    m_datumElevation.xmlCapability()->setIOWritable(false);
    m_datumElevation.xmlCapability()->setIOReadable(false);

    CAF_PDM_InitFieldNoDefault(&m_unitSystem, "UnitSystem", "Unit System", "", "", "");
    m_unitSystem.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitField(&m_simWellName, "SimWellName", QString(""), "Well", "", "", "");
    CAF_PDM_InitField(&m_branchIndex, "SimBranchIndex", 0, "Branch", "", "", "");

    CAF_PDM_InitField(&m_showWellPathLabel,           "ShowWellPathLabel",    true,           "Show Well Path Label", "", "", "");

    CAF_PDM_InitField(&m_showWellPath,                "ShowWellPath",         true,           "Show Well Path", "", "", "");
    m_showWellPath.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_wellPathRadiusScaleFactor,   "WellPathRadiusScale", 1.0,             "Well Path Radius Scale", "", "", "");
    CAF_PDM_InitField(&m_wellPathColor,               "WellPathColor",       cvf::Color3f(0.999f, 0.333f, 0.999f), "Well Path Color", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_completions, "Completions", "Completions", "", "", "");
    m_completions = new RimWellPathCompletions;
    m_completions.uiCapability()->setUiTreeHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_wellLogFiles, "WellLogFiles", "Well Log Files", "", "", "");
    m_wellLogFiles.uiCapability()->setUiTreeHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_3dWellLogCurves, "CollectionOf3dWellLogCurves", "3D Track", "", "", "");
    m_3dWellLogCurves = new Rim3dWellLogCurveCollection;
    m_3dWellLogCurves.uiCapability()->setUiTreeHidden(true);

    CAF_PDM_InitField(&m_formationKeyInFile, "WellPathFormationKeyInFile", QString(""), "Key in File", "", "", "");
    m_formationKeyInFile.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitField(&m_wellPathFormationFilePath, "WellPathFormationFilePath", QString(""), "File Path", "", "", "");
    m_wellPathFormationFilePath.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitFieldNoDefault(&m_wellLogFile_OBSOLETE,      "WellLogFile",  "Well Log File", "", "", "");
    m_wellLogFile_OBSOLETE.uiCapability()->setUiHidden(true);
    m_wellLogFile_OBSOLETE.xmlCapability()->setIOWritable(false);

    m_wellPath = nullptr;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPath::~RimWellPath()
{
    if (m_wellLogFile_OBSOLETE())
    {
        delete m_wellLogFile_OBSOLETE;
    }

    for(const auto& file : m_wellLogFiles())
    {
        delete file;
    }

    RimProject* project;
    firstAncestorOrThisOfType(project);
    if (project)
    {
        if (project->mainPlotCollection())
        {
            RimWellLogPlotCollection* plotCollection = project->mainPlotCollection()->wellLogPlotCollection();
            if (plotCollection)
            {
                plotCollection->removeExtractors(m_wellPath.p());
            }
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWellPath::userDescriptionField()
{
    return &m_name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimWellPath::wellPathRadius(double characteristicCellSize) const
{
    double radius = characteristicCellSize * m_wellPathRadiusScaleFactor();

    RimWellPathCollection* coll = nullptr;
    this->firstAncestorOrThisOfType(coll);
    if (coll)
    {
        radius *= coll->wellPathRadiusScaleFactor();
    }

    return radius;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimWellPath::wellPathRadiusScaleFactor() const
{
    return m_wellPathRadiusScaleFactor();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFishbonesCollection* RimWellPath::fishbonesCollection()
{
    CVF_ASSERT(m_completions);

    return m_completions->fishbonesCollection();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RimFishbonesCollection * RimWellPath::fishbonesCollection() const
{
    CVF_ASSERT(m_completions);

    return m_completions->fishbonesCollection();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimPerforationCollection* RimWellPath::perforationIntervalCollection()
{
    CVF_ASSERT(m_completions);

    return m_completions->perforationCollection();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RimPerforationCollection* RimWellPath::perforationIntervalCollection() const
{
    CVF_ASSERT(m_completions);

    return m_completions->perforationCollection();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RimWellPathCompletions* RimWellPath::completions() const
{
    return m_completions();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPathFractureCollection* RimWellPath::fractureCollection()
{
    CVF_ASSERT(m_completions);

    return m_completions->fractureCollection();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RimWellPathFractureCollection * RimWellPath::fractureCollection() const
{
    CVF_ASSERT(m_completions);

    return m_completions->fractureCollection();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigWellPath* RimWellPath::wellPathGeometry()
{
    return m_wellPath.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RigWellPath* RimWellPath::wellPathGeometry() const
{
    return m_wellPath.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPath::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimProject* proj;
    this->firstAncestorOrThisOfTypeAsserted(proj);
    if (changedField == &m_showWellPath)
    {
        proj->reloadCompletionTypeResultsInAllViews();
    }
    else
    {
        proj->createDisplayModelAndRedrawAllViews();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellPath::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_simWellName)
    {
        RimProject* proj = RiaApplication::instance()->project();

        // Find simulation wells already assigned to a well path
        std::set<QString> associatedSimWells;
        for (const auto& wellPath : proj->allWellPaths())
        {
            if (wellPath->isAssociatedWithSimulationWell() && wellPath != this)
            {
                associatedSimWells.insert(wellPath->associatedSimulationWellName());
            }
        }

        options.push_back(caf::PdmOptionItemInfo(SIM_WELL_NONE_UI_TEXT, ""));
        for (const auto& wellName : proj->simulationWellNames())
        {
            if (associatedSimWells.count(wellName) > 0) continue;

            options.push_back(caf::PdmOptionItemInfo(wellName, wellName));
        }
    }
    else if (fieldNeedingOptions == &m_branchIndex)
    {
        size_t branchCount = RimWellPath::simulationWellBranchCount(m_simWellName);

        if (branchCount == 0)
            branchCount = 1;

        size_t index = 0;
        while(index < branchCount)
        {
            QString uiText = QString("Branch %1").arg(QString::number(index + 1));
            options.push_back(caf::PdmOptionItemInfo(uiText, QVariant::fromValue(index)));
            index++;
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPath::initAfterRead()
{
    RimWellLogFile* wellLogFile = m_wellLogFile_OBSOLETE();
    m_wellLogFile_OBSOLETE = nullptr;

    if (wellLogFile != nullptr)
    {
        m_wellLogFiles.push_back(wellLogFile);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellPath::name() const
{
    return m_name();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPath::setName(const QString& name)
{
    m_name = name;
    m_completions->setWellNameForExport(name);
    tryAssociateWithSimulationWell();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimWellLogFile*> RimWellPath::wellLogFiles() const
{
    return std::vector<RimWellLogFile*>(m_wellLogFiles.begin(), m_wellLogFiles.end());
}
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellPath::showWellPathLabel() const
{
    return m_showWellPathLabel();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellPath::showWellPath() const
{
    return m_showWellPath();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimWellPath::wellPathColor() const
{
    return m_wellPathColor;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPath::setWellPathColor(const cvf::Color3f& color)
{
    m_wellPathColor = color;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWellPath::objectToggleField()
{
    return &m_showWellPath;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPath::setWellPathGeometry(RigWellPath* wellPathModel)
{
    m_wellPath = wellPathModel;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPath::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    if (m_simWellName().isEmpty())
    {
        // Try to set default simulation well name
        tryAssociateWithSimulationWell();
    }

    caf::PdmUiGroup* appGroup =  uiOrdering.addNewGroup("Appearance");
    appGroup->add(&m_showWellPathLabel);
    appGroup->add(&m_wellPathColor);
    appGroup->add(&m_wellPathRadiusScaleFactor); 


    caf::PdmUiGroup* simWellGroup = uiOrdering.addNewGroup("Simulation Well");
    simWellGroup->add(&m_simWellName);

    if (simulationWellBranchCount(m_simWellName) > 1)
    {
        simWellGroup->add(&m_branchIndex);
    }

    caf::PdmUiGroup* ssihubGroup =  uiOrdering.addNewGroup("Well Info");

    ssihubGroup->add(&m_datumElevation);
    ssihubGroup->add(&m_unitSystem);

    if (m_wellPath.notNull() && m_wellPath->hasDatumElevation())
    {
        m_datumElevation = m_wellPath->datumElevation();
        m_datumElevation.uiCapability()->setUiHidden(false);
    }
    else
    {
        m_datumElevation.uiCapability()->setUiHidden(true);
    }

    caf::PdmUiGroup* formationFileInfoGroup = uiOrdering.addNewGroup("Well Picks");
    formationFileInfoGroup->add(&m_wellPathFormationFilePath);
    formationFileInfoGroup->add(&m_formationKeyInFile);

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPath::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName)
{ 
    uiTreeOrdering.add(&m_wellLogFiles);

    if (m_completions->hasCompletions())
    {
        uiTreeOrdering.add(&m_completions);
    }

    if (m_3dWellLogCurves->has3dWellLogCurves())
    {
        uiTreeOrdering.add(&m_3dWellLogCurves);
    }

    uiTreeOrdering.skipRemainingChildren(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RimWellPath::simulationWellBranchCount(const QString& simWellName)
{
    bool detectBranches = true;

    auto branches = RiaSimWellBranchTools::simulationWellBranches(simWellName, detectBranches);

    return branches.size();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPath::updateFilePathsFromProjectPath(const QString& newProjectPath, const QString& oldProjectPath)
{

    {
        bool                 foundFile = false;
        std::vector<QString> searchedPaths;

        QString fileNameCandidate = RimTools::relocateFile(m_wellPathFormationFilePath, newProjectPath, oldProjectPath, &foundFile, &searchedPaths);
        if (foundFile)
        {
            m_wellPathFormationFilePath = fileNameCandidate;
        }
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimWellPath::combinedScaleFactor() const
{
    RimWellPathCollection* wellPathColl = nullptr;
    this->firstAncestorOrThisOfTypeAsserted(wellPathColl);

    return this->m_wellPathRadiusScaleFactor() * wellPathColl->wellPathRadiusScaleFactor();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPath::setUnitSystem(RiaEclipseUnitTools::UnitSystem unitSystem)
{
    m_unitSystem = unitSystem;

    m_completions->setUnitSystemSpecificDefaults();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaEclipseUnitTools::UnitSystem RimWellPath::unitSystem() const
{
    return m_unitSystem();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPath::addWellLogFile(RimWellLogFile* logFileInfo)
{
    // Prevent the same file from being loaded more than once
    auto itr = std::find_if(m_wellLogFiles.begin(), m_wellLogFiles.end(), [&](const RimWellLogFile* file) 
    { 
        return QString::compare(file->fileName(), logFileInfo->fileName(), Qt::CaseInsensitive) == 0;
    });

    // Todo: Verify well name to ensure all well log files having the same well name

    if (itr == m_wellLogFiles.end())
    {
        m_wellLogFiles.push_back(logFileInfo);

        if (m_wellLogFiles.size() == 1 && name().isEmpty())
        {
            setName(m_wellLogFiles[0]->wellName());
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPath::deleteWellLogFile(RimWellLogFile* logFileInfo)
{
    detachWellLogFile(logFileInfo);
    delete logFileInfo;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPath::detachWellLogFile(RimWellLogFile* logFileInfo)
{
    auto pdmObject = dynamic_cast<caf::PdmObjectHandle*>(logFileInfo);
    for (size_t i = 0; i < m_wellLogFiles.size(); i++)
    {
        if (m_wellLogFiles[i] == pdmObject)
        {
            m_wellLogFiles.removeChildObject(pdmObject);
            break;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPath::setFormationsGeometry(cvf::ref<RigWellPathFormations> wellPathFormations)
{
    m_wellPathFormations = wellPathFormations;
    m_wellPathFormationFilePath = wellPathFormations->filePath();
    m_formationKeyInFile = wellPathFormations->keyInFile();

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellPath::readWellPathFormationsFile(QString* errorMessage, RifWellPathFormationsImporter* wellPathFormationsImporter)
{
    if (m_wellPathFormationFilePath().isEmpty())
    {
        return true;
    }

    if (caf::Utils::fileExists(m_wellPathFormationFilePath()))
    {
        m_wellPathFormations = wellPathFormationsImporter->readWellPathFormations(m_wellPathFormationFilePath(), m_formationKeyInFile());
        if (m_name().isEmpty())
        {
            setName(m_formationKeyInFile());
        }
        return true;
    }
    else
    {
        if (errorMessage) (*errorMessage) = "Could not find the well pick file: " + m_wellPathFormationFilePath();
        return false;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellPath::reloadWellPathFormationsFile(QString* errorMessage, RifWellPathFormationsImporter* wellPathFormationsImporter)
{
    if (m_wellPathFormationFilePath().isEmpty())
    {
        return true;
    }

    if (caf::Utils::fileExists(m_wellPathFormationFilePath()))
    {
        m_wellPathFormations = wellPathFormationsImporter->reloadWellPathFormations(m_wellPathFormationFilePath(), m_formationKeyInFile());
        return true;
    }
    else
    {
        if (errorMessage) (*errorMessage) = "Could not find the well pick file: " + m_wellPathFormationFilePath();
        return false;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellPath::hasFormations() const
{
    if (m_wellPathFormations.isNull())
    {
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RigWellPathFormations* RimWellPath::formationsGeometry() const
{
    return m_wellPathFormations.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPath::add3dWellLogCurve(Rim3dWellLogCurve* rim3dWellLogCurve)
{
    m_3dWellLogCurves->add3dWellLogCurve(rim3dWellLogCurve);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Rim3dWellLogCurveCollection* RimWellPath::rim3dWellLogCurveCollection() const
{
    return m_3dWellLogCurves();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const QString RimWellPath::associatedSimulationWellName() const
{
    return m_simWellName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RimWellPath::associatedSimulationWellBranch() const
{
    return m_branchIndex;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellPath::tryAssociateWithSimulationWell()
{
    if (!m_simWellName().isEmpty()) return false;

    QString matchedSimWell = RiaWellNameComparer::tryFindMatchingSimWellName(m_name);
    
    if (!matchedSimWell.isEmpty())
    {
        m_simWellName = matchedSimWell;
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellPath::isAssociatedWithSimulationWell() const
{
    return !m_simWellName().isEmpty();
}
