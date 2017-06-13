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

#include "RifWellPathImporter.h"

#include "RigWellPath.h"

#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimTools.h"
#include "RimWellLogFile.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellPathCollection.h"

#include "RimFishbonesMultipleSubs.h"
#include "RimWellPathCompletions.h"

#include "RiuMainWindow.h"

#include "RivWellPathPartMgr.h"

#include "cafPdmUiTreeOrdering.h"
#include "cafUtils.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>

CAF_PDM_SOURCE_INIT(RimWellPath, "WellPath");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPath::RimWellPath()
{
    CAF_PDM_InitObject("WellPath", ":/Well.png", "", "");

    CAF_PDM_InitFieldNoDefault(&name,               "WellPathName",                         "Name", "", "", "");
    name.uiCapability()->setUiReadOnly(true);
    name.xmlCapability()->setIOWritable(false);
    name.xmlCapability()->setIOReadable(false);
    name.uiCapability()->setUiHidden(true);
    CAF_PDM_InitFieldNoDefault(&id,                 "WellPathId",                           "Id", "", "", "");
    id.uiCapability()->setUiReadOnly(true);
    id.xmlCapability()->setIOWritable(false);
    id.xmlCapability()->setIOReadable(false);
    CAF_PDM_InitFieldNoDefault(&sourceSystem,       "SourceSystem",                         "Source System", "", "", "");
    sourceSystem.uiCapability()->setUiReadOnly(true);
    sourceSystem.xmlCapability()->setIOWritable(false);
    sourceSystem.xmlCapability()->setIOReadable(false);
    CAF_PDM_InitFieldNoDefault(&utmZone,            "UTMZone",                              "UTM Zone", "", "", "");
    utmZone.uiCapability()->setUiReadOnly(true);
    utmZone.xmlCapability()->setIOWritable(false);
    utmZone.xmlCapability()->setIOReadable(false);
    CAF_PDM_InitFieldNoDefault(&updateDate,         "WellPathUpdateDate",                   "Update Date", "", "", "");
    updateDate.uiCapability()->setUiReadOnly(true);
    updateDate.xmlCapability()->setIOWritable(false);
    updateDate.xmlCapability()->setIOReadable(false);
    CAF_PDM_InitFieldNoDefault(&updateUser,         "WellPathUpdateUser",                   "Update User", "", "", "");
    updateUser.uiCapability()->setUiReadOnly(true);
    updateUser.xmlCapability()->setIOWritable(false);
    updateUser.xmlCapability()->setIOReadable(false);
    CAF_PDM_InitFieldNoDefault(&m_surveyType,       "WellPathSurveyType",                   "Survey Type", "", "", "");
    m_surveyType.uiCapability()->setUiReadOnly(true);
    m_surveyType.xmlCapability()->setIOWritable(false);
    m_surveyType.xmlCapability()->setIOReadable(false);

    CAF_PDM_InitFieldNoDefault(&m_datumElevation, "DatumElevation", "Datum Elevation", "", "", "");
    m_datumElevation.uiCapability()->setUiReadOnly(true);
    m_datumElevation.xmlCapability()->setIOWritable(false);
    m_datumElevation.xmlCapability()->setIOReadable(false);

    CAF_PDM_InitFieldNoDefault(&m_unitSystem, "UnitSystem", "Unit System", "", "", "");
    m_unitSystem.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitField(&filepath,                    "WellPathFilepath",     QString(""),    "Filepath", "", "", "");
    filepath.uiCapability()->setUiReadOnly(true);
    CAF_PDM_InitField(&wellPathIndexInFile,         "WellPathNumberInFile",     -1,    "Well Number in file", "", "", "");
    wellPathIndexInFile.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitField(&showWellPathLabel,           "ShowWellPathLabel",    true,           "Show well path label", "", "", "");

    CAF_PDM_InitField(&showWellPath,                "ShowWellPath",         true,           "Show well path", "", "", "");
    showWellPath.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&wellPathRadiusScaleFactor,   "WellPathRadiusScale", 1.0,             "Well path radius scale", "", "", "");
    CAF_PDM_InitField(&wellPathColor,               "WellPathColor",       cvf::Color3f(0.999f, 0.333f, 0.999f), "Well path color", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_completions, "Completions", "Completions", "", "", "");
    m_completions = new RimWellPathCompletions;
    m_completions.uiCapability()->setUiTreeHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_wellLogFile,      "WellLogFile",  "Well Log File", "", "", "");
    m_wellLogFile.uiCapability()->setUiHidden(true);

    m_wellPath = NULL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPath::~RimWellPath()
{
    if (m_wellLogFile())
    {
        delete m_wellLogFile;
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
    return &name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPath::setSurveyType(QString surveyType) 
{ 
    m_surveyType = surveyType; 
    if (m_surveyType == "PLAN")
        wellPathColor = cvf::Color3f(0.999f, 0.333f, 0.0f);
    else if (m_surveyType == "PROTOTYPE")
        wellPathColor = cvf::Color3f(0.0f, 0.333f, 0.999f);
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
RigWellPath* RimWellPath::wellPathGeometry()
{
    return m_wellPath.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RigWellPath * RimWellPath::wellPathGeometry() const
{
    return m_wellPath.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivWellPathPartMgr* RimWellPath::partMgr()
{
    if (m_wellPathPartMgr.isNull()) 
    {
        RimWellPathCollection* wpColl;
        this->firstAncestorOrThisOfType(wpColl);
        if (wpColl) m_wellPathPartMgr = new RivWellPathPartMgr(this);
    }

    return m_wellPathPartMgr.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPath::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    partMgr()->scheduleGeometryRegen();

    RimProject* proj;
    this->firstAncestorOrThisOfTypeAsserted(proj);
    if (changedField == &showWellPath)
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
caf::PdmFieldHandle* RimWellPath::objectToggleField()
{
    return &showWellPath;
}

//--------------------------------------------------------------------------------------------------
/// Read JSON or ascii file containing well path data
//--------------------------------------------------------------------------------------------------
bool RimWellPath::readWellPathFile(QString* errorMessage, RifWellPathImporter* wellPathImporter)
{
    if (caf::Utils::fileExists(filepath()))
    {
        RifWellPathImporter::WellData wellData = wellPathImporter->readWellData(filepath(), wellPathIndexInFile());
        RifWellPathImporter::WellMetaData wellMetaData = wellPathImporter->readWellMetaData(filepath(), wellPathIndexInFile());
        // General well info

        name = wellData.m_name;
        id = wellMetaData.m_id;
        sourceSystem = wellMetaData.m_sourceSystem;
        utmZone = wellMetaData.m_utmZone;
        updateUser = wellMetaData.m_updateUser;
        setSurveyType(wellMetaData.m_surveyType);
        updateDate = wellMetaData.m_updateDate.toString("d MMMM yyyy");

        m_wellPath = wellData.m_wellPathGeometry;
        return true;
    }
    else
    {
        if (errorMessage) (*errorMessage) = "Could not find the well path file: " + filepath();
        return false;
    }
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
    caf::PdmUiGroup* appGroup =  uiOrdering.addNewGroup("Appearance");
    appGroup->add(&showWellPathLabel);
    appGroup->add(&wellPathColor);
    appGroup->add(&wellPathRadiusScaleFactor); 

    caf::PdmUiGroup* fileInfoGroup =   uiOrdering.addNewGroup("File");
    fileInfoGroup->add(&filepath);
    fileInfoGroup->add(&wellPathIndexInFile);

    caf::PdmUiGroup* ssihubGroup =  uiOrdering.addNewGroup("Well Info");
    ssihubGroup->add(&id);
    ssihubGroup->add(&sourceSystem);
    ssihubGroup->add(&utmZone);
    ssihubGroup->add(&updateDate);
    ssihubGroup->add(&updateUser);
    ssihubGroup->add(&m_surveyType);
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
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPath::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName)
{ 
    uiTreeOrdering.skipRemainingChildren(true);
    uiTreeOrdering.add(&m_wellLogFile);
    uiTreeOrdering.add(&m_completions);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellPath::getCacheDirectoryPath()
{
    QString cacheDirPath = RimTools::getCacheRootDirectoryPathFromProject();
    cacheDirPath += "_wellpaths";
    return cacheDirPath;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellPath::getCacheFileName()
{
    if (filepath().isEmpty())
    {
        return "";
    }

    QString cacheFileName;

    // Make the path correct related to the possibly new project filename
    QString newCacheDirPath = getCacheDirectoryPath();
    QFileInfo oldCacheFile(filepath);

   
    cacheFileName = newCacheDirPath + "/" + oldCacheFile.fileName();

    return cacheFileName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPath::setupBeforeSave()
{
    // SSIHUB is the only source for populating Id, use text in this field to decide if the cache file must be copied to new project cache location
    if (!isStoredInCache())
    {
        return;
    }

    if (filepath().isEmpty())
    {
        return;
    }

    QDir::root().mkpath(getCacheDirectoryPath());

    QString newCacheFileName = getCacheFileName();

    // Use QFileInfo to get same string representation to avoid issues with mix of forward and backward slashes
    QFileInfo prevFileInfo(filepath);
    QFileInfo currentFileInfo(newCacheFileName);

    if (prevFileInfo.absoluteFilePath().compare(currentFileInfo.absoluteFilePath()) != 0)
    {
        QFile::copy(filepath, newCacheFileName);

        filepath = newCacheFileName;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellPath::isStoredInCache()
{
    // SSIHUB is the only source for populating Id, use text in this field to decide if the cache file must be copied to new project cache location
    return !id().isEmpty();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPath::updateFilePathsFromProjectPath(const QString& newProjectPath, const QString& oldProjectPath)
{
    if (isStoredInCache())
    {
        QString newCacheFileName = getCacheFileName();

        if (caf::Utils::fileExists(newCacheFileName))
        {
            filepath = newCacheFileName;
        }
    }
    else
    {
        filepath = RimTools::relocateFile(filepath(), newProjectPath, oldProjectPath, NULL, NULL);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimWellPath::combinedScaleFactor() const
{
    RimWellPathCollection* wellPathColl = nullptr;
    this->firstAncestorOrThisOfTypeAsserted(wellPathColl);

    return this->wellPathRadiusScaleFactor() * wellPathColl->wellPathRadiusScaleFactor();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPath::setUnitSystem(RiaEclipseUnitTools::UnitSystem unitSystem)
{
    m_unitSystem = unitSystem;
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
void RimWellPath::setLogFileInfo(RimWellLogFile* logFileInfo)
{
    if (m_wellLogFile())
    {
        delete m_wellLogFile;
    }

    m_wellLogFile = logFileInfo;
    m_wellLogFile->uiCapability()->setUiHidden(true);

    this->name = m_wellLogFile->wellName();
}
