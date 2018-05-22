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

#include "RiuPlotMainWindowTools.h"
#include "RiuPlotMainWindow.h"

#include "cafUtils.h"
#include "cafPdmSettings.h"
#include "cafPdmUiPropertyViewDialog.h"
#include "cafPdmUiObjectEditorHandle.h"

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
    caf::PdmUiObjectEditorHandle::updateUiAllObjectEditors();
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
std::vector<RimSummaryCase*> RimObservedDataCollection::allObservedData()
{
    std::vector<RimSummaryCase*> allObservedData;

    allObservedData.insert(allObservedData.begin(), m_observedDataArray.begin(), m_observedDataArray.end());

    return allObservedData;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimObservedDataCollection::fileExists(const QString& fileName, QString* errorText /*= nullptr*/)
{
    QFile file(fileName);
    if (!file.exists())
    {
        QString s = QString("File does not exist, %1").arg(fileName);
        RiaLogging::error(s);

        if (errorText) errorText->append(s);
        return false;
    }
    return true;
}

void updateNewSummaryObjectCreated(caf::PdmObject* object)
{
    RiuPlotMainWindowTools::showPlotMainWindow();
    RiuPlotMainWindowTools::selectAsCurrentItem(object);
    RiuPlotMainWindowTools::setExpanded(object);

    caf::PdmUiObjectEditorHandle::updateUiAllObjectEditors();

    RiuPlotMainWindow* mpw = RiaApplication::instance()->mainPlotWindow();
    if (mpw) mpw->updateSummaryPlotToolBar();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimObservedData* RimObservedDataCollection::createAndAddRsmObservedDataFromFile(const QString& fileName, QString* errorText /*= nullptr*/)
{
    if (!fileExists(fileName, errorText)) return nullptr;

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

    updateNewSummaryObjectCreated(observedData);

    this->updateConnectedEditors();

    return observedData;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimObservedData* RimObservedDataCollection::createAndAddCvsObservedDataFromFile(const QString& fileName, bool useSavedFieldsValuesInDialog, QString* errorText /*= nullptr*/)
{
    if (!fileExists(fileName, errorText)) return nullptr;

    RimObservedData* observedData = nullptr;
    bool parseOk = false;

    RimCsvUserData* userData = new RimCsvUserData();
    RicPasteAsciiDataToSummaryPlotFeatureUi* parseOptions = userData->parseOptions();

    if (useSavedFieldsValuesInDialog)
    {
        caf::PdmSettings::readFieldsFromApplicationStore(parseOptions);
    }
    parseOptions->setUiModeImport(fileName);

    caf::PdmUiPropertyViewDialog propertyDialog(nullptr, parseOptions, "CSV Import Options", "");
    if (propertyDialog.exec() != QDialog::Accepted)
    {
        return nullptr;
    }

    caf::PdmSettings::writeFieldsToApplicationStore(parseOptions);

    //userData->setParseOptions(parseOptionsUi.parseOptions());
    userData->setSummaryHeaderFileName(fileName);
    userData->createSummaryReaderInterface();
    userData->updateMetaData();
    userData->updateOptionSensitivity();

    if (errorText && !userData->errorMessagesFromReader().isEmpty())
    {
        errorText->append(userData->errorMessagesFromReader());
    }

    if (userData->summaryReader())
    {
        this->m_observedDataArray.push_back(userData);
        observedData = userData;
        parseOk = true;
    }
    else
    {
        delete userData;
        return nullptr;
    }

    updateNewSummaryObjectCreated(observedData);

    this->updateConnectedEditors();

    return observedData;
}
