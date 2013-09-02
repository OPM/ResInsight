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

#include "RimWellPathImport.h"

namespace caf {

    template<>
    void caf::AppEnum< RimWellPathImport::UtmFilterEnum >::setUp()
    {
        addItem(RimWellPathImport::UTM_FILTER_OFF,      "UTM_FILTER_OFF",       "Off");
        addItem(RimWellPathImport::UTM_FILTER_PROJECT,  "UTM_FILTER_PROJECT",   "Project");
        addItem(RimWellPathImport::UTM_FILTER_CUSTOM,   "UTM_FILTER_CUSTOM",    "Custom");
        setDefault(RimWellPathImport::UTM_FILTER_PROJECT);
    }

    template<>
    void caf::AppEnum< RimWellPathImport::WellTypeEnum >::setUp()
    {
        addItem(RimWellPathImport::WELL_ALL,     "WELL_ALL",     "All");
        addItem(RimWellPathImport::WELL_SURVEY,  "WELL_SURVEY",  "Survey");
        addItem(RimWellPathImport::WELL_PLAN,    "WELL_PLAN",    "Plan");
        setDefault(RimWellPathImport::WELL_ALL);
    }

} // End namespace caf


CAF_PDM_SOURCE_INIT(RimWellPathImport, "RimWellPathImport");



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPathImport::RimWellPathImport()
{
    CAF_PDM_InitObject("RimWellPathImport", "", "", "");

    caf::AppEnum<RimWellPathImport::WellTypeEnum> defaultWellMode = WELL_ALL;
    CAF_PDM_InitField(&wellMode, "WellMode", defaultWellMode, "Well types",   "", "", "");
 
    caf::AppEnum<RimWellPathImport::UtmFilterEnum> defaultUtmMode = UTM_FILTER_OFF;
    CAF_PDM_InitField(&utmFilterMode, "UtmMode", defaultUtmMode, "Utm filter",   "", "", "");

    CAF_PDM_InitField(&north, "UtmNorth", 0.0,    "North", "", "", "");
    CAF_PDM_InitField(&south, "UtmSouth", 0.0,    "South", "", "", "");
    CAF_PDM_InitField(&east,  "UtmEast",  0.0,    "East", "", "", "");
    CAF_PDM_InitField(&west,  "UtmWest",  0.0,    "West", "", "", "");

    CAF_PDM_InitFieldNoDefault(&regions, "Regions", "",  "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathImport::updateRegions(const QStringList& regionStrings, const QStringList& fieldStrings, const QStringList& edmIds)
{
    assert(regionStrings.size() == fieldStrings.size() && regionStrings.size() == edmIds.size());

    for (int i = 0; i < regionStrings.size(); i++)
    {
        RimOilRegionEntry* oilRegionEntry = NULL;
        bool edmIdFound = false;

        for (size_t regionIdx = 0; regionIdx < this->regions.size(); regionIdx++)
        {
            if (this->regions[regionIdx]->name == regionStrings[i])
            {
                oilRegionEntry = this->regions[regionIdx];

                for (size_t fIdx = 0; fIdx < this->regions[regionIdx]->fields.size(); fIdx++)
                {
                    if (this->regions[regionIdx]->fields[fIdx]->edmId == edmIds[i])
                    {
                        edmIdFound = true;
                    }
                }
            }
        }

        if (!edmIdFound)
        {
            if (!oilRegionEntry)
            {
                oilRegionEntry = new RimOilRegionEntry;
                oilRegionEntry->name = regionStrings[i];

                this->regions.push_back(oilRegionEntry);
            }

            RimOilFieldEntry* oilFieldEntry = new RimOilFieldEntry;
            oilFieldEntry->name = fieldStrings[i];
            oilFieldEntry->edmId = edmIds[i];

            oilRegionEntry->fields.push_back(oilFieldEntry);
        }
    }
}

