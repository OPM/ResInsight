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

#include "RimOilFieldEntry.h"
#include "RimWellPathImport.h"

#include "RifJsonEncodeDecode.h"

#include <QFile>
#include <QFileInfo>
#include <QMap>

CAF_PDM_SOURCE_INIT(RimOilFieldEntry, "RimOilFieldEntry");





//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimOilFieldEntry::RimOilFieldEntry()
{
    CAF_PDM_InitObject("OilFieldEntry", "", "", "");

    CAF_PDM_InitFieldNoDefault(&name,       "OilFieldName",      "OilFieldName", "", "", "");
    CAF_PDM_InitFieldNoDefault(&edmId,      "EdmId",             "EdmId", "", "", "");

    CAF_PDM_InitField(&selected,       "Selected",         true,   "Selected", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimOilFieldEntry::userDescriptionField()
{
    return &name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimOilFieldEntry::objectToggleField()
{
    return &selected;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimOilFieldEntry::parseWellsResponse(const QString& absolutePath, const QString& wsAddress)
{
    wells.deleteAllChildObjects();

    if (QFile::exists(wellsFilePath))
    {
        JsonReader jsonReader;
        QMap<QString, QVariant> jsonMap = jsonReader.decodeFile(wellsFilePath);

        QMapIterator<QString, QVariant> it(jsonMap);
        while (it.hasNext())
        {
            it.next();

            QString key = it.key();
            if (key[0].isDigit())
            {
                QMap<QString, QVariant> rootMap = it.value().toMap();
                QMap<QString, QVariant> surveyMap = rootMap["survey"].toMap();

                {
                    QString name = surveyMap["name"].toString();
                    QMap<QString, QVariant> linksMap = surveyMap["links"].toMap();
                    QString requestUrl = wsAddress + linksMap["entity"].toString();
                    QString surveyType = surveyMap["surveyType"].toString();
                    RimWellPathEntry* surveyWellPathEntry = RimWellPathEntry::createWellPathEntry(name, surveyType, requestUrl, absolutePath, RimWellPathEntry::WELL_SURVEY);
                    wells.push_back(surveyWellPathEntry);
                }

                QMap<QString, QVariant> plansMap = rootMap["plans"].toMap();
                QMapIterator<QString, QVariant> planIt(plansMap);
                while (planIt.hasNext())
                {
                    planIt.next();
                    QString name = surveyMap["name"].toString();
                    QMap<QString, QVariant> linksMap = surveyMap["links"].toMap();
                    QString requestUrl = wsAddress + linksMap["entity"].toString();
                    QString surveyType = surveyMap["surveyType"].toString();
                    RimWellPathEntry* surveyWellPathEntry = RimWellPathEntry::createWellPathEntry(name, surveyType, requestUrl, absolutePath, RimWellPathEntry::WELL_PLAN);
                    wells.push_back(surveyWellPathEntry);
                }
            }
        }
    }

}

