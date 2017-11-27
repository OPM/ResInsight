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

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "SummaryPlotCommands/RicPasteAsciiDataToSummaryPlotFeatureUi.h"

#include "RifKeywordVectorParser.h"

#include "RimObservedData.h"
#include "RimCsvUserData.h"
#include "RimObservedEclipseUserData.h"
#include "RimSummaryObservedDataFile.h"

#include "RiuMainPlotWindow.h"

#include "cafUtils.h"
#include "cafPdmSettings.h"
#include "cafPdmUiPropertyViewDialog.h"

#include <QFile>

CAF_PDM_SOURCE_INIT(RimObservedDataCollection, "ObservedDataCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimObservedDataCollection::RimObservedDataCollection()
{
    CAF_PDM_InitObject("Observed Time History Data", ":/Folder.png", "", "");
    
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
RimObservedData* RimObservedDataCollection::createAndAddObservedDataFromFileName(const QString& fileName, QString* errorText)
{
    RimObservedData* observedData = nullptr;

    {
        QFile file(fileName);
        if (!file.exists())
        {
            QString s = QString("File does not exist, %1").arg(fileName);
            RiaLogging::error(s);

            if (errorText) errorText->append(s);
            return nullptr;
        }
    }

    if (fileName.endsWith(".rsm", Qt::CaseInsensitive))
    {
        return createAndAddRsmObservedDataFromFile(fileName, errorText);
    }
    else if (fileName.endsWith(".txt", Qt::CaseInsensitive) || fileName.endsWith(".csv", Qt::CaseInsensitive))
    {
        return createAndAddCvsObservedDataFromFile(fileName, errorText);
    }
    else
    {
        if (errorText)
        {
            errorText->append("Not able to import file. Make sure '*.rsm' is used as extension if data is in RMS format or '*.txt' or '*.csv' if data is in CSV format.");
        }
    }
    
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*> RimObservedDataCollection::allObservedData()
{
    std::vector<RimSummaryCase*> allObservedData;

    allObservedData.insert(allObservedData.begin(), m_observedDataArray.begin(), m_observedDataArray.end());

    return allObservedData;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimObservedData* RimObservedDataCollection::createAndAddRsmObservedDataFromFile(const QString& fileName, QString* errorText /*= nullptr*/)
{
    RimObservedData* observedData = nullptr;
    RimObservedEclipseUserData* columnBasedUserData = new RimObservedEclipseUserData();
    observedData = columnBasedUserData;

    this->m_observedDataArray.push_back(observedData);
    observedData->setSummaryHeaderFileName(fileName);
    observedData->createSummaryReaderInterface();
    observedData->updateMetaData();
    observedData->updateOptionSensitivity();

    if (errorText && !observedData->errorMessagesFromReader().isEmpty())
    {
        errorText->append(observedData->errorMessagesFromReader());
    }

    RiuMainPlotWindow* mainPlotWindow = RiaApplication::instance()->getOrCreateAndShowMainPlotWindow();
    if (mainPlotWindow)
    {
        mainPlotWindow->selectAsCurrentItem(observedData);
        mainPlotWindow->setExpanded(observedData);
    }

    this->updateConnectedEditors();

    return observedData;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimObservedData* RimObservedDataCollection::createAndAddCvsObservedDataFromFile(const QString& fileName, QString* errorText /*= nullptr*/)
{
    RimObservedData* observedData = nullptr;

    RicPasteAsciiDataToSummaryPlotFeatureUi parseOptionsUi;
    parseOptionsUi.setUiModeImport(fileName);

    caf::PdmUiPropertyViewDialog propertyDialog(NULL, &parseOptionsUi, "CSV Import Options", "");
    if (propertyDialog.exec() != QDialog::Accepted)
    {
        return nullptr;
    }

    RimCsvUserData* columnBasedUserData = new RimCsvUserData();
    columnBasedUserData->setParseOptions(parseOptionsUi.parseOptions());
    observedData = columnBasedUserData;

    observedData->setSummaryHeaderFileName(fileName);
    observedData->createSummaryReaderInterface();
    if (observedData->summaryReader())
    {
        this->m_observedDataArray.push_back(observedData);
    }
    else
    {
        delete columnBasedUserData;
        return nullptr;
    }
    observedData->updateMetaData();
    observedData->updateOptionSensitivity();

    if (errorText && !observedData->errorMessagesFromReader().isEmpty())
    {
        errorText->append(observedData->errorMessagesFromReader());
    }

    RiuMainPlotWindow* mainPlotWindow = RiaApplication::instance()->getOrCreateAndShowMainPlotWindow();
    if (mainPlotWindow)
    {
        mainPlotWindow->selectAsCurrentItem(observedData);
        mainPlotWindow->setExpanded(observedData);
    }

    this->updateConnectedEditors();
    return observedData;
}
