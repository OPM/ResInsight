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

#include "RimInputPropertyCollection.h"


CAF_PDM_SOURCE_INIT(RimInputPropertyCollection, "RimInputPropertyCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimInputPropertyCollection::RimInputPropertyCollection()
{
    CAF_PDM_InitObject("Input Properties", ":/EclipseInput48x48.png", "", "");

    CAF_PDM_InitFieldNoDefault(&inputProperties, "InputProperties", "",  "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimInputPropertyCollection::~RimInputPropertyCollection()
{

}

//--------------------------------------------------------------------------------------------------
/// Returns the InputProperties pointing to the same file as \a fileName
//--------------------------------------------------------------------------------------------------
std::vector<RimInputProperty*> RimInputPropertyCollection::findInputProperties(QString fileName)
{
    QFileInfo fileInfo(fileName);
    std::vector<RimInputProperty*> result;
    for (size_t i = 0; i < this->inputProperties.size(); ++i)
    {
        if (!inputProperties[i]) continue;

        QFileInfo propFile(inputProperties[i]->fileName());
        if (fileInfo == propFile) result.push_back(inputProperties[i]);
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/// Remove given input property from collection and checks if the associated file is referenced by other input
/// properties
//--------------------------------------------------------------------------------------------------
void RimInputPropertyCollection::removeInputProperty(RimInputProperty* inputProperty, bool& isPropertyFileReferencedByOthers)
{
    CVF_ASSERT(inputProperty);

    this->inputProperties.removeChildObject(inputProperty);

    isPropertyFileReferencedByOthers = false;
    for (size_t i = 0; i < this->inputProperties.size(); i++)
    {
        if (inputProperties[i]->fileName() == inputProperty->fileName)
        {
            isPropertyFileReferencedByOthers = true;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Returns the InputProperty with resultName  \a resultName
//--------------------------------------------------------------------------------------------------
RimInputProperty * RimInputPropertyCollection::findInputProperty(QString resultName)
{
    for (size_t i = 0; i < this->inputProperties.size(); i++)
    {
        if (inputProperties[i] && inputProperties[i]->resultName() == resultName)
        {
            return inputProperties[i];
        }
    }

    return NULL;
}

