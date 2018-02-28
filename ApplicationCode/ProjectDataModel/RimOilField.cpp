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

#include "RimOilField.h"

#include "RimEclipseCaseCollection.h"
#include "RimFormationNamesCollection.h"
#include "RimFractureTemplateCollection.h"
#include "RimGeoMechModels.h"
#include "RimObservedData.h"
#include "RimObservedDataCollection.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimWellPathCollection.h"


CAF_PDM_SOURCE_INIT(RimOilField, "ResInsightOilField");
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimOilField::RimOilField(void)
{
    CAF_PDM_InitObject("Oil Field", "", "", "");

    CAF_PDM_InitFieldNoDefault(&analysisModels, "AnalysisModels", "Grid Models", ":/GridModels.png", "", "");
    CAF_PDM_InitFieldNoDefault(&geoMechModels, "GeoMechModels", "Geo Mech Models", ":/GridModels.png", "", "");
    CAF_PDM_InitFieldNoDefault(&wellPathCollection, "WellPathCollection", "Well Paths", ":/WellCollection.png", "", "");

    CAF_PDM_InitFieldNoDefault(&fractureDefinitionCollection, "FractureDefinitionCollection", "Defenition of Fractures", "", "", "");

    CAF_PDM_InitFieldNoDefault(&summaryCaseMainCollection,"SummaryCaseCollection","Summary Cases",":/GridModels.png","","");
    CAF_PDM_InitFieldNoDefault(&formationNamesCollection,"FormationNamesCollection","Formations","","","");
    CAF_PDM_InitFieldNoDefault(&observedDataCollection, "ObservedDataCollection", "Observed Data", ":/Cases16x16.png", "", "");

    fractureDefinitionCollection = new RimFractureTemplateCollection();
    analysisModels = new RimEclipseCaseCollection();
    wellPathCollection = new RimWellPathCollection();
    summaryCaseMainCollection = new RimSummaryCaseMainCollection();
    observedDataCollection = new RimObservedDataCollection();
    formationNamesCollection = new RimFormationNamesCollection();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimOilField::~RimOilField(void)
{
    if (wellPathCollection()) delete wellPathCollection();

    if (fractureDefinitionCollection()) delete fractureDefinitionCollection();
    if (geoMechModels()) delete geoMechModels();
    if (analysisModels()) delete analysisModels();
    if (summaryCaseMainCollection()) delete summaryCaseMainCollection();
    if (formationNamesCollection()) delete formationNamesCollection();
    if (observedDataCollection()) delete observedDataCollection();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimOilField::uniqueShortNameForCase(RimSummaryCase* summaryCase)
{
    std::set<QString> allAutoShortNames;

    std::vector<RimSummaryCase*> allCases = summaryCaseMainCollection->allSummaryCases();
    std::vector<RimSummaryCase*> observedDataCases = observedDataCollection->allObservedData();
    
    for (auto observedData : observedDataCases)
    {
        allCases.push_back(dynamic_cast<RimSummaryCase*>(observedData));
    }
    
    for (RimSummaryCase* sumCase : allCases)
    {
        if (sumCase && sumCase != summaryCase)
        {
            allAutoShortNames.insert(sumCase->shortName());
        }
    }

    bool foundUnique = false;

    QString caseName = summaryCase->caseName();
    QString shortName;

    if (caseName.size() > 2)
    {
        QString candidate;
        candidate += caseName[0];

        for (int i = 1; i < caseName.size(); ++i)
        {
            if (allAutoShortNames.count(candidate + caseName[i]) == 0)
            {
                shortName = candidate + caseName[i];
                foundUnique = true;
                break;
            }
        }
    }
    else
    {
        shortName = caseName.left(2);
        if (allAutoShortNames.count(shortName) == 0)
        {
            foundUnique = true;
        }
    }

    QString candidate = shortName;
    int autoNumber = 0;

    while (!foundUnique)
    {
        candidate = shortName + QString::number(autoNumber++);
        if (allAutoShortNames.count(candidate) == 0)
        {
            shortName = candidate;
            foundUnique = true;
        }
    }

    return shortName;
}

