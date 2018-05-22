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

#include "RimEclipseInputPropertyCollection.h"

#include "RimEclipseInputProperty.h"

#include <QFileInfo>

CAF_PDM_SOURCE_INIT(RimEclipseInputPropertyCollection, "RimInputPropertyCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseInputPropertyCollection::RimEclipseInputPropertyCollection()
{
    CAF_PDM_InitObject("Input Properties", ":/EclipseInput48x48.png", "", "");

    CAF_PDM_InitFieldNoDefault(&inputProperties, "InputProperties", "",  "", "", "");
    inputProperties.uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseInputPropertyCollection::~RimEclipseInputPropertyCollection()
{
    inputProperties.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// Returns the InputProperties pointing to the same file as \a fileName
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipseInputProperty*> RimEclipseInputPropertyCollection::findInputProperties(QString fileName)
{
    QFileInfo fileInfo(fileName);
    std::vector<RimEclipseInputProperty*> result;
    for (size_t i = 0; i < this->inputProperties.size(); ++i)
    {
        if (!inputProperties[i]) continue;

        QFileInfo propFile(inputProperties[i]->fileName());
        if (fileInfo == propFile) result.push_back(inputProperties[i]);
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/// Returns the InputProperty with resultName  \a resultName
//--------------------------------------------------------------------------------------------------
RimEclipseInputProperty * RimEclipseInputPropertyCollection::findInputProperty(QString resultName)
{
    for (size_t i = 0; i < this->inputProperties.size(); i++)
    {
        if (inputProperties[i] && inputProperties[i]->resultName() == resultName)
        {
            return inputProperties[i];
        }
    }

    return nullptr;
}

