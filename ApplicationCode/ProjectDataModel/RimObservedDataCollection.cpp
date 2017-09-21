/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017- Statoil ASA
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


#include "RimObservedDataCollection.h"
#include "RimObservedData.h"
#include "RimSummaryObservedDataFile.h"

CAF_PDM_SOURCE_INIT(RimObservedDataCollection, "ObservedDataCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimObservedDataCollection::RimObservedDataCollection()
{
    CAF_PDM_InitObject("Observed Data", ":/Folder.png", "", "");
    
    CAF_PDM_InitFieldNoDefault(&m_observedDataArray, "ObservedDataArray", "", "", "", "");

    m_observedDataArray.uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimObservedDataCollection::~RimObservedDataCollection()
{
    m_observedDataArray.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimObservedDataCollection::removeObservedData(RimObservedData* observedData)
{
    m_observedDataArray.removeChildObject(observedData);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimObservedDataCollection::addObservedData(RimObservedData* observedData)
{
    m_observedDataArray.push_back(observedData);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryObservedDataFile* RimObservedDataCollection::createAndAddObservedDataFromFileName(const QString& fileName)
{
    RimSummaryObservedDataFile* newObservedData = new RimSummaryObservedDataFile();

    this->m_observedDataArray.push_back(newObservedData);
    newObservedData->setSummaryHeaderFilename(fileName);
    newObservedData->updateOptionSensitivity();

    this->updateConnectedEditors();

    return newObservedData;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimObservedData*> RimObservedDataCollection::allObservedData()
{
    std::vector<RimObservedData*> allObservedData;

    allObservedData.insert(allObservedData.begin(), m_observedDataArray.begin(), m_observedDataArray.end());

    return allObservedData;
}
