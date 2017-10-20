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

#include "RimWellPathCollection.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"
#include "RiaColorTables.h"

#include "RigWellPath.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"

#include "RimProject.h"
#include "RimWellLogFile.h"
#include "RimWellPath.h"
#include "RimOilField.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"

#include "RiuMainWindow.h"

#include "RifWellPathImporter.h"

#include "cafPdmUiEditorHandle.h"
#include "cafProgressInfo.h"

#include <QFile>
#include <QFileInfo>
#include <QMessageBox>

#include <fstream>
#include <cmath>
#include "RivWellPathPartMgr.h"
#include "RimEclipseView.h"

namespace caf
{
    template<>
    void RimWellPathCollection::WellVisibilityEnum::setUp()
    {
        addItem(RimWellPathCollection::FORCE_ALL_OFF,       "FORCE_ALL_OFF",      "Off");
        addItem(RimWellPathCollection::ALL_ON,              "ALL_ON",             "Individual");
        addItem(RimWellPathCollection::FORCE_ALL_ON,        "FORCE_ALL_ON",       "On");
    }
}


CAF_PDM_SOURCE_INIT(RimWellPathCollection, "WellPaths");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPathCollection::RimWellPathCollection()
{
    CAF_PDM_InitObject("Wells", ":/WellCollection.png", "", "");

    CAF_PDM_InitField(&isActive,              "Active",        true,   "Active", "", "", "");
    isActive.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&showWellPathLabel,               "ShowWellPathLabel",        true,                       "Show well path labels", "", "", "");

    cvf::Color3f defWellLabelColor = RiaApplication::instance()->preferences()->defaultWellLabelColor();
    CAF_PDM_InitField(&wellPathLabelColor,              "WellPathLabelColor",   defWellLabelColor, "Well label color",  "", "", "");

    CAF_PDM_InitField(&wellPathVisibility,              "GlobalWellPathVisibility", WellVisibilityEnum(ALL_ON), "Global well path visibility",  "", "", "");

    CAF_PDM_InitField(&wellPathRadiusScaleFactor,       "WellPathRadiusScale",      0.1,                        "Well Path radius scale", "", "", "");
    CAF_PDM_InitField(&wellPathCrossSectionVertexCount, "WellPathVertexCount",      12,                          "Well Path vertex count", "", "", "");
    wellPathCrossSectionVertexCount.xmlCapability()->setIOWritable(false);
    wellPathCrossSectionVertexCount.xmlCapability()->setIOReadable(false);
    wellPathCrossSectionVertexCount.uiCapability()->setUiHidden(true);
    CAF_PDM_InitField(&wellPathClip,                    "WellPathClip",             true,                       "Clip Well Paths", "", "", "");
    CAF_PDM_InitField(&wellPathClipZDistance,           "WellPathClipZDistance",    100,                        "Well path clipping depth distance", "", "", "");

    CAF_PDM_InitFieldNoDefault(&wellPaths,              "WellPaths",                                            "Well Paths",  "", "", "");

    wellPaths.uiCapability()->setUiHidden(true);

    m_wellPathImporter = new RifWellPathImporter;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPathCollection::~RimWellPathCollection()
{
   wellPaths.deleteAllChildObjects();
   delete m_wellPathImporter;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    scheduleRedrawAffectedViews();
}


//--------------------------------------------------------------------------------------------------
/// Read JSON files containing well path data
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::readWellPathFiles()
{
    caf::ProgressInfo progress(wellPaths.size(), "Reading well paths from file");

    for (size_t wpIdx = 0; wpIdx < wellPaths.size(); wpIdx++)
    {
        if (!wellPaths[wpIdx]->filepath().isEmpty())
        {
            QString errorMessage;
            if (!wellPaths[wpIdx]->readWellPathFile(&errorMessage, m_wellPathImporter))
            {
                QMessageBox::warning(RiuMainWindow::instance(),
                                     "File open error",
                                     errorMessage);
            }
        }

        for (RimWellLogFile* const wellLogFile : wellPaths[wpIdx]->wellLogFiles())
        {
            if (wellLogFile)
            {
                QString errorMessage;
                if (!wellLogFile->readFile(&errorMessage))
                {
                    QString displayMessage = "Could not open the well log file: \n" + wellLogFile->fileName();

                    if (!errorMessage.isEmpty())
                    {
                        displayMessage += "\n\n";
                        displayMessage += errorMessage;
                    }

                    QMessageBox::warning(RiuMainWindow::instance(),
                                         "File open error",
                                         displayMessage);
                }
            }
        }
        progress.setProgressDescription(QString("Reading file %1").arg(wellPaths[wpIdx]->name()));
        progress.incrementProgress();
    }

    this->sortWellsByName();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::addWellPaths( QStringList filePaths )
{
    std::vector<RimWellPath*> wellPathArray;

    for (QString filePath : filePaths)
    {
        // Check if this file is already open
        bool alreadyOpen = false;
        for (size_t wpIdx = 0; wpIdx < wellPaths.size(); wpIdx++)
        {
            QFile f1;
            f1.setFileName(filePath);
            QString s1 = f1.fileName();
            QFile f2;
            f2.setFileName(wellPaths[wpIdx]->filepath());
            QString s2 = f2.fileName();
            if (s1 == s2)
            {
                //printf("Attempting to open well path JSON file that is already open:\n  %s\n", (const char*) filePath.toLocal8Bit());
                alreadyOpen = true;
                break;
            }
        }

        if (!alreadyOpen)
        {
            QFileInfo fi(filePath);

            if (fi.suffix().compare("json") == 0)
            {
                RimWellPath* wellPath = new RimWellPath();
                wellPath->filepath = filePath;
                wellPathArray.push_back(wellPath);
            }
            else
            {
                // Create Well path objects for all the paths in the assumed ascii file
                size_t wellPathCount = m_wellPathImporter->wellDataCount(filePath);
                for (size_t i = 0; i < wellPathCount; ++i)
                {
                    RimWellPath* wellPath = new RimWellPath();
                    wellPath->filepath = filePath;
                    wellPath->wellPathIndexInFile = static_cast<int>(i);
                    wellPathArray.push_back(wellPath);
                }
            }
        }
    }

    readAndAddWellPaths(wellPathArray);

    RimProject* proj;
    firstAncestorOrThisOfTypeAsserted(proj);
    proj->reloadCompletionTypeResultsInAllViews();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::readAndAddWellPaths(std::vector<RimWellPath*>& wellPathArray)
{
    caf::ProgressInfo progress(wellPathArray.size(), "Reading well paths from file");

    const caf::ColorTable& colorTable = RiaColorTables::wellLogPlotPaletteColors();
    cvf::Color3ubArray wellColors = colorTable.color3ubArray();
    cvf::Color3ubArray interpolatedWellColors = wellColors;

    if (wellPathArray.size() > 1)
    {
        interpolatedWellColors = caf::ColorTable::interpolateColorArray(wellColors, wellPathArray.size());
    }

    for (size_t wpIdx = 0; wpIdx < wellPathArray.size(); wpIdx++)
    {
        RimWellPath* wellPath = wellPathArray[wpIdx];
        wellPath->readWellPathFile(NULL, m_wellPathImporter);

        progress.setProgressDescription(QString("Reading file %1").arg(wellPath->name()));

        // If a well path with this name exists already, make it read the well path file
        RimWellPath* existingWellPath = wellPathByName(wellPath->name());
        if (existingWellPath)
        {
            existingWellPath->filepath = wellPath->filepath;
            existingWellPath->wellPathIndexInFile = wellPath->wellPathIndexInFile;
            existingWellPath->readWellPathFile(NULL, m_wellPathImporter);

            delete wellPath;
        }
        else
        {
            wellPath->wellPathColor = cvf::Color3f(interpolatedWellColors[wpIdx]);
            wellPath->setUnitSystem(findUnitSystemForWellPath(wellPath));
            wellPaths.push_back(wellPath);
        }

        progress.incrementProgress();
    }

    this->sortWellsByName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::addWellPaths(const std::vector<RimWellPath*> wellPaths)
{
    for(const auto& wellPath : wellPaths)
    {
        this->wellPaths.push_back(wellPath);
    }
    this->sortWellsByName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogFile* RimWellPathCollection::addWellLogs(const QStringList& filePaths)
{
    RimWellLogFile* logFileInfo;

    foreach (QString filePath, filePaths)
    {
        logFileInfo = RimWellLogFile::readWellLogFile(filePath);
        if (logFileInfo)
        {
            RimWellPath* wellPath = wellPathByName(logFileInfo->wellName());
            if (!wellPath)
            {
                wellPath = new RimWellPath();
                wellPaths.push_back(wellPath);
            }

            wellPath->addWellLogFile(logFileInfo);
        }
    }

    this->sortWellsByName();

    return logFileInfo;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* wellHeadGroup = uiOrdering.addNewGroup("Well labels");
    wellHeadGroup->add(&showWellPathLabel);
    wellHeadGroup->add(&wellPathLabelColor);

    caf::PdmUiGroup* wellPipe = uiOrdering.addNewGroup("Well pipe");
    wellPipe->add(&wellPathVisibility);
    wellPipe->add(&wellPathRadiusScaleFactor);

    caf::PdmUiGroup* advancedGroup = uiOrdering.addNewGroup("Clipping");
    advancedGroup->add(&wellPathClip);
    advancedGroup->add(&wellPathClipZDistance);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWellPathCollection::objectToggleField()
{
    return &isActive;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::scheduleRedrawAffectedViews()
{
    RimProject* proj;
    this->firstAncestorOrThisOfType(proj);
    if (proj) proj->createDisplayModelAndRedrawAllViews();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::appendStaticGeometryPartsToModel(cvf::ModelBasicList* model,
                                                             double characteristicCellSize,
                                                             const cvf::BoundingBox& wellPathClipBoundingBox,
                                                             const caf::DisplayCoordTransform* displayCoordTransform)
{
    if (!this->isActive()) return;
    if (this->wellPathVisibility() == RimWellPathCollection::FORCE_ALL_OFF) return;

    for (size_t wIdx = 0; wIdx < this->wellPaths.size(); wIdx++)
    {
        RivWellPathPartMgr* partMgr = this->wellPaths[wIdx]->partMgr();
        partMgr->appendStaticGeometryPartsToModel(model, characteristicCellSize, wellPathClipBoundingBox, displayCoordTransform);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#ifdef USE_PROTOTYPE_FEATURE_FRACTURES
void RimWellPathCollection::appendStaticFracturePartsToModel(cvf::ModelBasicList* model, 
                                                             const RimEclipseView* eclView)
{
    if (!this->isActive()) return;
    if (this->wellPathVisibility() == RimWellPathCollection::FORCE_ALL_OFF) return;

    for (size_t wIdx = 0; wIdx < this->wellPaths.size(); wIdx++)
    {
        RivWellPathPartMgr* partMgr = this->wellPaths[wIdx]->partMgr();
        partMgr->appendStaticFracturePartsToModel(model, eclView);
    }
}
#endif // USE_PROTOTYPE_FEATURE_FRACTURES

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::appendDynamicGeometryPartsToModel(cvf::ModelBasicList* model,
                                                              const QDateTime& timeStamp,
                                                              double characteristicCellSize,
                                                              const cvf::BoundingBox& wellPathClipBoundingBox,
                                                              const caf::DisplayCoordTransform* displayCoordTransform)

{
    if (!this->isActive()) return;
    if (this->wellPathVisibility() == RimWellPathCollection::FORCE_ALL_OFF) return;

    for (size_t wIdx = 0; wIdx < this->wellPaths.size(); wIdx++)
    {
        RivWellPathPartMgr* partMgr = this->wellPaths[wIdx]->partMgr();
        partMgr->appendDynamicGeometryPartsToModel(model, timeStamp, characteristicCellSize, wellPathClipBoundingBox, displayCoordTransform);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::updateFilePathsFromProjectPath(const QString& newProjectPath, const QString& oldProjectPath)
{
    for (size_t wellPathIdx = 0; wellPathIdx < wellPaths.size(); wellPathIdx++)
    {
        wellPaths[wellPathIdx]->updateFilePathsFromProjectPath(newProjectPath, oldProjectPath);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPath* RimWellPathCollection::wellPathByName(const QString& wellPathName) const
{
    for (size_t wellPathIdx = 0; wellPathIdx < wellPaths.size(); wellPathIdx++)
    {
        if (wellPaths[wellPathIdx]->name() == wellPathName)
        {
            return wellPaths[wellPathIdx];
        }
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::deleteAllWellPaths()
{
    wellPaths.deleteAllChildObjects();

    m_wellPathImporter->clear();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::removeWellPath(RimWellPath* wellPath)
{
    wellPaths.removeChildObject(wellPath);

    bool isFilePathUsed = false;
    for (size_t i = 0; i < wellPaths.size(); i++)
    {
        if (wellPaths[i]->filepath == wellPath->filepath)
        {
            isFilePathUsed = true;
            break;
        }
    }

    if (!isFilePathUsed)
    {
        // One file can have multiple well paths
        // If no other well paths are referencing the filepath, remove cached data from the file reader
        m_wellPathImporter->removeFilePath(wellPath->filepath);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool lessWellPath(const caf::PdmPointer<RimWellPath>& w1,  const caf::PdmPointer<RimWellPath>& w2)
{
    if (w1.notNull() && w2.notNull())
        return (w1->name() < w2->name());
    else if (w1.notNull())
        return true;
    else 
        return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::sortWellsByName()
{
    std::sort(wellPaths.begin(), wellPaths.end(), lessWellPath);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaEclipseUnitTools::UnitSystemType RimWellPathCollection::findUnitSystemForWellPath(const RimWellPath* wellPath)
{
    RimProject* project;
    firstAncestorOrThisOfTypeAsserted(project);
    if (project->activeOilField()->analysisModels->cases.empty())
    {
        return RiaEclipseUnitTools::UNITS_UNKNOWN;
    }

    const RigEclipseCaseData* eclipseCaseData = project->activeOilField()->analysisModels->cases()[0]->eclipseCaseData();
    cvf::BoundingBox caseBoundingBox = eclipseCaseData->mainGrid()->boundingBox();
    cvf::BoundingBox wellPathBoundingBox;
    for (auto& wellPathPoint : wellPath->wellPathGeometry()->m_wellPathPoints)
    {
        wellPathBoundingBox.add(wellPathPoint);
    }

    if (caseBoundingBox.intersects(wellPathBoundingBox))
    {
        return eclipseCaseData->unitsType();
    }
    return RiaEclipseUnitTools::UNITS_UNKNOWN;
}
