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

#ifdef USE_PROTOTYPE_FEATURE_FRACTURES
#include "RimFractureTemplateCollection.h"
#endif // USE_PROTOTYPE_FEATURE_FRACTURES

#include "RimGeoMechModels.h"
#include "RimSummaryCaseCollection.h"
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

#ifdef USE_PROTOTYPE_FEATURE_FRACTURES
    CAF_PDM_InitFieldNoDefault(&fractureDefinitionCollection, "FractureDefinitionCollection", "Defenition of fractures", "", "", "");
#endif // USE_PROTOTYPE_FEATURE_FRACTURES

    CAF_PDM_InitFieldNoDefault(&summaryCaseCollection,"SummaryCaseCollection","Summary Cases",":/GridModels.png","","");
    CAF_PDM_InitFieldNoDefault(&formationNamesCollection,"FormationNamesCollection","Formations","","","");

#ifdef USE_PROTOTYPE_FEATURE_FRACTURES
    fractureDefinitionCollection = new RimFractureTemplateCollection();
#endif // USE_PROTOTYPE_FEATURE_FRACTURES

    analysisModels = new RimEclipseCaseCollection();
    wellPathCollection = new RimWellPathCollection();
    summaryCaseCollection = new RimSummaryCaseCollection();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimOilField::~RimOilField(void)
{
    if (wellPathCollection()) delete wellPathCollection();

#ifdef USE_PROTOTYPE_FEATURE_FRACTURES
    if (fractureDefinitionCollection()) delete fractureDefinitionCollection();
#endif // USE_PROTOTYPE_FEATURE_FRACTURES

    if (geoMechModels()) delete geoMechModels();
    if (analysisModels()) delete analysisModels();
    if (summaryCaseCollection()) delete summaryCaseCollection();
    if (formationNamesCollection()) delete formationNamesCollection();
}

