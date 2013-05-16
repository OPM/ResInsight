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

    CAF_PDM_InitField(&wellPathRadiusScaleFactor,       "WellPathRadiusScale",      0.5,                        "Well Path radius scale", "", "", "");
    CAF_PDM_InitField(&wellPathCrossSectionVertexCount, "WellPathVertexCount",      12,                          "Pipe vertex count", "", "", "");
    wellPathCrossSectionVertexCount.setIOWritable(false);
    wellPathCrossSectionVertexCount.setIOReadable(false);
    wellPathCrossSectionVertexCount.setUiHidden(true);
    CAF_PDM_InitField(&wellPathClip,                    "WellPathClip",             true,                       "Clip Well Paths", "", "", "");
    CAF_PDM_InitField(&wellPathClipZDistance,           "WellPathClipZDistance",    500,                        "Well path clipping depth distance", "", "", "");

    CAF_PDM_InitFieldNoDefault(&wellPaths,              "WellPaths",                                            "Well Paths",  "", "", "");

    m_wellPathCollectionPartManager = new RivWellPathCollectionPartMgr(this);
    m_project = NULL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPathCollection::~RimWellPathCollection()
{
   wellPaths.deleteAllChildObjects();
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
                printf("Attempting to open well path JSON file that is already open:\n  %s\n", (const char*) filePath.toLocal8Bit());
                alreadyOpen = true;
                break;
            }
        }

        if (!alreadyOpen)
        {
            RimWellPath* wellPath = new RimWellPath();
            wellPath->setProject(m_project);
            wellPath->setCollection(this);
            wellPath->filepath = filePath;
            wellPaths.push_back(wellPath);
        }
    }

    readWellPathFiles();
}
