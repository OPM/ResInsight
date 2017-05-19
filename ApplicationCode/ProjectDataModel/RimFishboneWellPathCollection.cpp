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

#include "RimFishboneWellPathCollection.h"

#include "RimEclipseWell.h"
#include "RimFishboneWellPath.h"
#include "RimView.h"
#include "RimProject.h"

#include "RigWellPath.h"

#include "RifWellPathImporter.h"

#include "RiuMainWindow.h"


CAF_PDM_SOURCE_INIT(RimFishboneWellPathCollection, "WellPathCompletionCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFishboneWellPathCollection::RimFishboneWellPathCollection()
{
    CAF_PDM_InitObject("WellPathCompletions", ":/WellCollection.png", "", "");

    m_name.uiCapability()->setUiHidden(true);
    m_name = "Well Paths";

    CAF_PDM_InitFieldNoDefault(&m_wellPaths, "WellPaths", "Well Paths", "", "", "");
    m_wellPaths.uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFishboneWellPathCollection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimProject* proj;
    this->firstAncestorOrThisOfTypeAsserted(proj);
    proj->createDisplayModelAndRedrawAllViews();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFishboneWellPathCollection::appendCompletion(RimFishboneWellPath* completion)
{
    m_wellPaths.push_back(completion);

    updateConnectedEditors();
    RiuMainWindow::instance()->selectAsCurrentItem(completion);

    RimView* rimView = NULL;
    firstAncestorOrThisOfType(rimView);
    if (rimView)
    {
        rimView->scheduleCreateDisplayModelAndRedraw();
    }

    uiCapability()->setUiHidden(!m_wellPaths.empty());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFishboneWellPathCollection::importCompletionsFromFile(const QStringList& filePaths)
{
    RifWellPathImporter wellPathImporter;

    foreach(const QString& filePath, filePaths) {
        size_t wellDataCount = wellPathImporter.wellDataCount(filePath);

        for (size_t i = 0; i < wellDataCount; ++i)
        {
            RifWellPathImporter::WellData wellData = wellPathImporter.readWellData(filePath, i);
            RimFishboneWellPath* wellCompletion = new RimFishboneWellPath();
            wellCompletion->setName(wellData.m_name);
            wellCompletion->setCoordinates(wellData.m_wellPathGeometry->m_wellPathPoints);
            wellCompletion->setMeasuredDepths(wellData.m_wellPathGeometry->m_measuredDepths);
            appendCompletion(wellCompletion);
        }
    }
}
