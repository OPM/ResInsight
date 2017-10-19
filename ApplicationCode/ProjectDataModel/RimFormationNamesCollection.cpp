/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
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

#include "RimFormationNamesCollection.h"

#include "RimFormationNames.h"

#include <QMessageBox>

CAF_PDM_SOURCE_INIT(RimFormationNamesCollection, "FormationNamesCollectionObject");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFormationNamesCollection::RimFormationNamesCollection()
{
    CAF_PDM_InitObject("Formations", ":/FormationCollection16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_formationNamesList, "FormationNamesList",  "Formations", "", "", "");
    m_formationNamesList.uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFormationNamesCollection::~RimFormationNamesCollection()
{
    m_formationNamesList.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFormationNamesCollection::readAllFormationNames()
{
    for(RimFormationNames* fmNames: m_formationNamesList)
    {
        fmNames->readFormationNamesFile(nullptr);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFormationNamesCollection::importFiles(const QStringList& fileNames)
{
    QStringList newFileNames;
    std::vector<RimFormationNames*> formNamesObjsToReload;

    for(const QString& newFileName : fileNames)
    {
        bool isFound = false;
        for(RimFormationNames* fmNames: m_formationNamesList)
        {
            if(fmNames->fileName() == newFileName)
            {
                formNamesObjsToReload.push_back(fmNames);
                isFound = true;
                break;
            }
        }

        if(!isFound)
        {
            newFileNames.push_back(newFileName);
        }
    }

    for(const QString& newFileName :  newFileNames)
    {
        RimFormationNames* newFNs = new RimFormationNames;
        newFNs->setFileName(newFileName);
        m_formationNamesList.push_back(newFNs);
        formNamesObjsToReload.push_back(newFNs);
    }

    QString totalErrorMessage;

    for (RimFormationNames* fmNames: formNamesObjsToReload)
    {
        QString errormessage;

        fmNames->readFormationNamesFile(&errormessage);
        if (!errormessage.isEmpty())
        {
            totalErrorMessage += "\nError in: " + fmNames->fileName() 
                               + "\n\t" + errormessage;
        }
    }

    if (!totalErrorMessage.isEmpty())
    {
        QMessageBox::warning(nullptr, "Import Formation Names", totalErrorMessage);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFormationNamesCollection::updateFilePathsFromProjectPath(const QString& newProjectPath, const QString& oldProjectPath)
{
    for(RimFormationNames* fmNames: m_formationNamesList)
    {
        fmNames->updateFilePathsFromProjectPath(newProjectPath, oldProjectPath);
    }
}
