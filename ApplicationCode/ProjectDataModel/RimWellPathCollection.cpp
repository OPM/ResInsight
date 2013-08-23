/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RiaStdInclude.h"

#include "cafAppEnum.h"
#include "cafPdmFieldCvfColor.h"

#include "RimWellPathCollection.h"
#include "RimWellPath.h"
#include "RivWellPathCollectionPartMgr.h"
#include "RimProject.h"
#include "RimCase.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimScriptCollection.h"
#include "RimReservoirView.h"
#include "Rim3dOverlayInfoConfig.h"
#include "RimReservoirCellResultsCacher.h"
#include "RimCaseCollection.h"
#include "RimResultSlot.h"
#include "RimCellEdgeResultSlot.h"
#include "RimCellRangeFilterCollection.h"
#include "RimCellPropertyFilterCollection.h"
#include "RimWellCollection.h"
#include "RimOilField.h"
#include "RimAnalysisModels.h"
#include <fstream>

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

    CAF_PDM_InitField(&showWellPathLabel,               "ShowWellPathLabel",        true,                       "Show well path labels", "", "", "");

    CAF_PDM_InitField(&wellPathVisibility,              "GlobalWellPathVisibility", WellVisibilityEnum(ALL_ON), "Global well path visibility",  "", "", "");

    CAF_PDM_InitField(&wellPathRadiusScaleFactor,       "WellPathRadiusScale",      0.1,                        "Well Path radius scale", "", "", "");
    CAF_PDM_InitField(&wellPathCrossSectionVertexCount, "WellPathVertexCount",      12,                          "Well Path vertex count", "", "", "");
    wellPathCrossSectionVertexCount.setIOWritable(false);
    wellPathCrossSectionVertexCount.setIOReadable(false);
    wellPathCrossSectionVertexCount.setUiHidden(true);
    CAF_PDM_InitField(&wellPathClip,                    "WellPathClip",             true,                       "Clip Well Paths", "", "", "");
    CAF_PDM_InitField(&wellPathClipZDistance,           "WellPathClipZDistance",    100,                        "Well path clipping depth distance", "", "", "");

    CAF_PDM_InitFieldNoDefault(&wellPaths,              "WellPaths",                                            "Well Paths",  "", "", "");

    m_wellPathCollectionPartManager = new RivWellPathCollectionPartMgr(this);
    m_project = NULL;

    m_asciiFileReader = new RimWellPathAsciiFileReader;
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
    m_wellPathCollectionPartManager->scheduleGeometryRegen();
    if (m_project) m_project->createDisplayModelAndRedrawAllViews();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::setProject( RimProject* project )
{
    m_project = project;
    for (size_t wellPathIdx = 0; wellPathIdx < wellPaths.size(); wellPathIdx++)
    {
        wellPaths[wellPathIdx]->setProject(m_project);
        wellPaths[wellPathIdx]->setCollection(this);
    }
}


//--------------------------------------------------------------------------------------------------
/// Read JSON files containing well path data
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::readWellPathFiles()
{
    for (size_t wpIdx = 0; wpIdx < wellPaths.size(); wpIdx++)
    {
        wellPaths[wpIdx]->readWellPathFile();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::addWellPaths( QStringList filePaths )
{
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
                wellPath->setProject(m_project);
                wellPath->setCollection(this);
                wellPath->filepath = filePath;
                wellPaths.push_back(wellPath);
            }
            else
            {
                // Create Well path objects for all the paths in the assumed ascii file
                size_t wellPathCount = this->m_asciiFileReader->wellDataCount(filePath);
                for (size_t i = 0; i < wellPathCount; ++i)
                {
                    RimWellPath* wellPath = new RimWellPath();
                    wellPath->setProject(m_project);
                    wellPath->setCollection(this);
                    wellPath->filepath = filePath;
                    wellPath->wellPathIndexInFile = static_cast<int>(i);
                    wellPaths.push_back(wellPath);
                }
            }
        }
    }

    readWellPathFiles();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathAsciiFileReader::readAllWellData(QString filePath)
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

    while(stream.good())
    {
        stream >> x;
        if (stream.good())
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
                if (!fileWellDataArray.size() )
                {
                    fileWellDataArray.push_back(WellData());
                    fileWellDataArray.back().m_wellPathGeometry = new RigWellPath();
                }

                cvf::Vec3d wellPoint(x, y, -tvd);
                fileWellDataArray.back().m_wellPathGeometry->m_wellPathPoints.push_back(wellPoint);

                x = HUGE_VAL;
                y = HUGE_VAL;
                tvd = HUGE_VAL;
                md = HUGE_VAL;
            }
        }
        else
        {
            // Could not read one double.
            // we assume there is a comment line or a well path description
            stream.clear();

            std::string line;
            std::getline(stream, line, '\n');
            size_t quoteStartIdx = line.find_first_of("'`���");
            size_t quoteEndIdx = line.find_last_of("'`���");

            std::string wellName;

            if (quoteStartIdx < line.size() -1 )
            {
                // Extract the text between the quotes
                wellName = line.substr(quoteStartIdx + 1, quoteEndIdx - 1 - quoteStartIdx);
            }
            else if (quoteStartIdx > line.length() && quoteEndIdx > line.length())
            {
                // Did not find any quotes
                // Look for keyword Name
                std::string lineLowerCase = line;
                transform(lineLowerCase.begin(), lineLowerCase.end(), lineLowerCase.begin(), ::tolower );

                std::string token = "name ";
                size_t firstNameIdx = lineLowerCase.find_first_of(token);
                if (firstNameIdx < lineLowerCase.length())
                {
                    wellName = line.substr(firstNameIdx + token.length());
                }
            }

            if (wellName.size() > 0)
            {
                // Create a new Well data 
                fileWellDataArray.push_back(WellData());
                fileWellDataArray.back().m_wellPathGeometry = new RigWellPath();

                fileWellDataArray.back().m_name = wellName.c_str();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPathAsciiFileReader::WellData RimWellPathAsciiFileReader::readWellData(QString filePath, int indexInFile)
{
    this->readAllWellData(filePath);

    std::map<QString, std::vector<WellData> >::iterator it = m_fileNameToWellDataGroupMap.find(filePath);

    CVF_ASSERT(it != m_fileNameToWellDataGroupMap.end());

    if (indexInFile < it->second.size())
    {
        return it->second[indexInFile];
    }
    else
    {
        // Error : The ascii well path file does not contain that many wellpaths
        return WellData();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RimWellPathAsciiFileReader::wellDataCount(QString filePath)
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
