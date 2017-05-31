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

#include "RigWellPath.h"

#include "RimProject.h"
#include "RimWellLogFile.h"
#include "RimWellPath.h"

#include "RiuMainWindow.h"

#include "RivWellPathCollectionPartMgr.h"

#include "cafPdmUiEditorHandle.h"
#include "cafProgressInfo.h"

#include <QFile>
#include <QFileInfo>
#include <QMessageBox>

#include <fstream>
#include <cmath>

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

    m_wellPathCollectionPartManager = new RivWellPathCollectionPartMgr(this);

    m_asciiFileReader = new RifWellPathAsciiFileReader;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPathCollection::~RimWellPathCollection()
{
   wellPaths.deleteAllChildObjects();
   delete m_asciiFileReader;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    scheduleGeometryRegenAndRedrawViews();
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
            if (!wellPaths[wpIdx]->readWellPathFile(&errorMessage, this->asciiFileReader()))
            {
                QMessageBox::warning(RiuMainWindow::instance(),
                                     "File open error",
                                     errorMessage);
            }
        }

        RimWellLogFile* wellLogFile = wellPaths[wpIdx]->m_wellLogFile;
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

        progress.setProgressDescription(QString("Reading file %1").arg(wellPaths[wpIdx]->name));
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

    foreach (QString filePath, filePaths)
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
                size_t wellPathCount = this->m_asciiFileReader->wellDataCount(filePath);
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
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::readAndAddWellPaths(std::vector<RimWellPath*>& wellPathArray)
{
    caf::ProgressInfo progress(wellPathArray.size(), "Reading well paths from file");

    for (size_t wpIdx = 0; wpIdx < wellPathArray.size(); wpIdx++)
    {
        RimWellPath* wellPath = wellPathArray[wpIdx];
        wellPath->readWellPathFile(NULL, this->asciiFileReader());

        progress.setProgressDescription(QString("Reading file %1").arg(wellPath->name));

        // If a well path with this name exists already, make it read the well path file
        RimWellPath* existingWellPath = wellPathByName(wellPath->name);
        if (existingWellPath)
        {
            existingWellPath->filepath = wellPath->filepath;
            existingWellPath->wellPathIndexInFile = wellPath->wellPathIndexInFile;
            existingWellPath->readWellPathFile(NULL, this->asciiFileReader());

            delete wellPath;
        }
        else
        {
            wellPaths.push_back(wellPath);
        }

        progress.incrementProgress();
    }

    this->sortWellsByName();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::addWellLogs(const QStringList& filePaths)
{
    foreach (QString filePath, filePaths)
    {
        RimWellLogFile* logFileInfo = RimWellLogFile::readWellLogFile(filePath);
        if (logFileInfo)
        {
            RimWellPath* wellPath = wellPathByName(logFileInfo->wellName());
            if (!wellPath)
            {
                wellPath = new RimWellPath();
                wellPaths.push_back(wellPath);
            }

            wellPath->setLogFileInfo(logFileInfo);
        }
    }

    this->sortWellsByName();
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
void RimWellPathCollection::scheduleGeometryRegenAndRedrawViews()
{
    m_wellPathCollectionPartManager->scheduleGeometryRegen();
    RimProject* proj;
    this->firstAncestorOrThisOfType(proj);
    if (proj) proj->createDisplayModelAndRedrawAllViews();
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

    m_asciiFileReader->clear();
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
        m_asciiFileReader->removeFilePath(wellPath->filepath);
    }
}

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
void RifWellPathAsciiFileReader::readAllWellData(QString filePath)
{
    std::map<QString, std::vector<WellData> >::iterator it = m_fileNameToWellDataGroupMap.find(filePath);

    // If we have the file in the map, assume it is already read.
    if (it != m_fileNameToWellDataGroupMap.end())
    {
        return;
    }

    // Create the data container
    std::vector<WellData>& fileWellDataArray = m_fileNameToWellDataGroupMap[filePath];

    std::ifstream stream(filePath.toLatin1().data());
    double x(HUGE_VAL), y(HUGE_VAL), tvd(HUGE_VAL), md(HUGE_VAL);

    bool hasReadWellPointInCurrentWell = false;

    while (stream.good())
    {
        // First check if we can read a number
        stream >> x;
        if (stream.good()) // If we can, assume this line is a well point entry
        {
            stream >> y >> tvd >> md;
            if (!stream.good())
            {
                // -999 or otherwise to few numbers before some word
                if (x != -999)
                {
                    // Error in file: missing numbers at this line

                }
                stream.clear();
            }
            else
            {
                if (!fileWellDataArray.size())
                {
                    fileWellDataArray.push_back(WellData());
                    fileWellDataArray.back().m_wellPathGeometry = new RigWellPath();
                }

                cvf::Vec3d wellPoint(x, y, -tvd);
                fileWellDataArray.back().m_wellPathGeometry->m_wellPathPoints.push_back(wellPoint);
                fileWellDataArray.back().m_wellPathGeometry->m_measuredDepths.push_back(md);

                x = HUGE_VAL;
                y = HUGE_VAL;
                tvd = HUGE_VAL;
                md = HUGE_VAL;

                hasReadWellPointInCurrentWell = true;
            }
        }
        else
        {
            // Could not read one double.
            // we assume there is a comment line or a well path description
            stream.clear();

            std::string line;
            std::getline(stream, line, '\n');
            // Skip possible comment lines (-- is used in eclipse, so Haakon Høgstøl considered it smart to skip these here as well)
            // The first "-" is eaten by the stream >> x above
            if (line.find("-") == 0 || line.find("#") == 0)
            {
                // Comment line, just ignore
            }
            else
            {
                // Find the first and the last position of any quotes (and do not care to match quotes)
                size_t quoteStartIdx = line.find_first_of("'`´’‘");
                size_t quoteEndIdx = line.find_last_of("'`´’‘");

                std::string wellName;
                bool haveAPossibleWellStart = false;

                if (quoteStartIdx < line.size() -1)
                {
                    // Extract the text between the quotes
                    wellName = line.substr(quoteStartIdx + 1, quoteEndIdx - 1 - quoteStartIdx);
                    haveAPossibleWellStart = true;
                }
                else if (quoteStartIdx > line.length())
                {
                    // We did not find any quotes

                    // Supported alternatives are 
                    // name <WellNameA>
                    // wellname: <WellNameA>
                    std::string lineLowerCase = line;
                    transform(lineLowerCase.begin(), lineLowerCase.end(), lineLowerCase.begin(), ::tolower);

                    std::string tokenName = "name";
                    std::size_t foundNameIdx = lineLowerCase.find(tokenName);
                    if (foundNameIdx != std::string::npos)
                    {
                        std::string tokenColon = ":";
                        std::size_t foundColonIdx = lineLowerCase.find(tokenColon, foundNameIdx);
                        if (foundColonIdx != std::string::npos)
                        {
                            wellName = line.substr(foundColonIdx + tokenColon.length());
                        }
                        else
                        {
                            wellName = line.substr(foundNameIdx + tokenName.length());
                        }

                        haveAPossibleWellStart = true;
                    }
                    else
                    {
                        // Interpret the whole line as the well name.

                        QString name = line.c_str();
                        if (!name.trimmed().isEmpty())
                        {
                            wellName = name.trimmed().toStdString();
                            haveAPossibleWellStart = true;
                        }
                    }
                }

                if (haveAPossibleWellStart)
                {
                    // Create a new Well data if we have read some data into the previous one.
                    // if not, just overwrite the name
                    if (hasReadWellPointInCurrentWell || fileWellDataArray.size() == 0)
                    {
                        fileWellDataArray.push_back(WellData());
                        fileWellDataArray.back().m_wellPathGeometry = new RigWellPath();
                    }

                    QString name = wellName.c_str();
                    if (!name.trimmed().isEmpty())
                    {
                        // Do not overwrite the name aquired from a line above, if this line is empty
                        fileWellDataArray.back().m_name = name.trimmed();
                    }
                    hasReadWellPointInCurrentWell = false;
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifWellPathAsciiFileReader::WellData RifWellPathAsciiFileReader::readWellData(QString filePath, int indexInFile)
{
    this->readAllWellData(filePath);

    std::map<QString, std::vector<WellData> >::iterator it = m_fileNameToWellDataGroupMap.find(filePath);

    CVF_ASSERT(it != m_fileNameToWellDataGroupMap.end());

    if (indexInFile < static_cast<int>(it->second.size()))
    {
        return it->second[indexInFile];
    }
    else
    {
        // Error : The ascii well path file does not contain that many well paths
        return WellData();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RifWellPathAsciiFileReader::wellDataCount(QString filePath)
{
    std::map<QString, std::vector<WellData> >::iterator it = m_fileNameToWellDataGroupMap.find(filePath);

    // If we have the file in the map, assume it is already read.
    if (it != m_fileNameToWellDataGroupMap.end())
    {
        return it->second.size();
    }

    this->readAllWellData(filePath);
    it = m_fileNameToWellDataGroupMap.find(filePath);
    CVF_ASSERT(it != m_fileNameToWellDataGroupMap.end());

    return it->second.size();;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifWellPathAsciiFileReader::clear()
{
    m_fileNameToWellDataGroupMap.clear();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifWellPathAsciiFileReader::removeFilePath(const QString& filePath)
{
    m_fileNameToWellDataGroupMap.erase(filePath);
}
