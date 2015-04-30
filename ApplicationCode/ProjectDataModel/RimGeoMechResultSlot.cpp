/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RimGeoMechResultSlot.h"
#include "RimGeoMechView.h"
#include "RimLegendConfig.h"
#include "RimDefines.h"
#include "RimGeoMechCase.h"
#include "RifGeoMechReaderInterface.h"

namespace caf {

template<>
void caf::AppEnum< RimGeoMechResultSlot::ResultPositionEnum >::setUp()
{
    addItem(RimGeoMechResultSlot::NODAL,            "NODAL",            "Nodal");
    addItem(RimGeoMechResultSlot::ELEMENT_NODAL,    "ELEMENT_NODAL",    "Element Nodal");
    addItem(RimGeoMechResultSlot::INTEGRATION_POINT,"INTEGRATION_POINT","Integration Point");
    setDefault(RimGeoMechResultSlot::NODAL);
}
}


CAF_PDM_SOURCE_INIT(RimGeoMechResultSlot, "GeoMechResultSlot");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechResultSlot::RimGeoMechResultSlot(void)
{
  
    CAF_PDM_InitObject("Color Result", ":/CellResult.png", "", "");

    CAF_PDM_InitFieldNoDefault(&legendConfig, "LegendDefinition", "Legend Definition", "", "", "");
    this->legendConfig = new RimLegendConfig();

    CAF_PDM_InitFieldNoDefault(&m_resultPositionType, "ResultPositionType" , "Result Position", "", "", "");
    CAF_PDM_InitField(&m_resultFieldName, "ResultFieldName", RimDefines::undefinedResultName(), "Field Name", "", "", "");
    CAF_PDM_InitField(&m_resultComponentName, "ResultComponentName", RimDefines::undefinedResultName(), "Component", "", "", "");

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechResultSlot::~RimGeoMechResultSlot(void)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimGeoMechResultSlot::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (m_reservoirView)
    {
        RimGeoMechCase* gmCase = m_reservoirView->geoMechCase();
        cvf::ref<RifGeoMechReaderInterface> reader = gmCase->readerInterface();
        if (reader.notNull())
        {
            std::map<std::string, std::vector<std::string> >  fieldCompNames;

            if (m_resultPositionType == NODAL)
            {
                fieldCompNames = reader->scalarNodeFieldAndComponentNames();
            }

            if (&m_resultFieldName == fieldNeedingOptions)
            {
                for (auto it = fieldCompNames.begin(); it != fieldCompNames.end(); ++it)
                {
                    options.push_back(caf::PdmOptionItemInfo(QString::fromStdString(it->first), QString::fromStdString(it->first)));
                }
            }

            if (&m_resultComponentName == fieldNeedingOptions)
            {
                auto fieldIt = fieldCompNames.find(m_resultFieldName().toAscii().data());
                if (fieldIt != fieldCompNames.end())
                {
                    for (auto compIt = fieldIt->second.begin(); compIt != fieldIt->second.end(); ++compIt)
                    {
                        options.push_back(caf::PdmOptionItemInfo(QString::fromStdString(*compIt), QString::fromStdString(*compIt)));
                    }
                }

                if (!options.size()) options.push_back(caf::PdmOptionItemInfo("Undefined", "Undefined"));
            }
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechResultSlot::setReservoirView(RimGeoMechView* ownerReservoirView)
{
    m_reservoirView = ownerReservoirView;
}
