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

#include "RimCrossSection.h"

#include "RiaApplication.h"

#include "RimEclipseView.h"
#include "RimEclipseWell.h"
#include "RimEclipseWellCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellPath.h"



namespace caf {

template<>
void caf::AppEnum< RimCrossSection::CrossSectionEnum >::setUp()
{
    addItem(RimCrossSection::CS_WELL_PATH,       "CS_WELL_PATH",       "Well Path");
    addItem(RimCrossSection::CS_SIMULATION_WELL, "CS_SIMULATION_WELL", "Simulation Well");
    addItem(RimCrossSection::CS_USER_DEFINED,    "CS_USER_DEFINED",    "User defined");
    setDefault(RimCrossSection::CS_WELL_PATH);
}

template<>
void caf::AppEnum< RimCrossSection::CrossSectionDirEnum >::setUp()
{
    addItem(RimCrossSection::CS_VERTICAL,   "CS_VERTICAL",      "Vertical");
    addItem(RimCrossSection::CS_HORIZONTAL, "CS_HORIZONTAL",    "Horizontal");
    setDefault(RimCrossSection::CS_VERTICAL);
}

} 


CAF_PDM_SOURCE_INIT(RimCrossSection, "CrossSection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCrossSection::RimCrossSection()
{
    CAF_PDM_InitObject("Cross Section", "", "", "");

    CAF_PDM_InitField(&name,        "UserDescription",  QString("Cross Section Name"), "Name", "", "", "");
    CAF_PDM_InitField(&isActive,    "Active",           true, "Active", "", "", "");
    isActive.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&wellPath,       "WellPath",         "Well Path", "", "", "");
    CAF_PDM_InitFieldNoDefault(&simulationWell, "SimulationWell",   "Simulation Well", "", "", "");
    CAF_PDM_InitFieldNoDefault(&type,           "Type",             "Type", "", "", "");
    CAF_PDM_InitFieldNoDefault(&direction,      "Direction",        "Direction", "", "", "");

    uiCapability()->setUiChildrenHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCrossSection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCrossSection::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&name);
    uiOrdering.add(&type);
    uiOrdering.add(&direction);

    if (type == CS_WELL_PATH)
    {
        uiOrdering.add(&wellPath);
    }
    else if (type == CS_SIMULATION_WELL)
    {
        uiOrdering.add(&simulationWell);
    }
    else
    {

    }

    uiOrdering.setForgetRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimCrossSection::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &wellPath)
    {
        RimProject* proj = RiaApplication::instance()->project();
        if (proj->activeOilField()->wellPathCollection())
        {
            caf::PdmChildArrayField<RimWellPath*>& wellPaths = proj->activeOilField()->wellPathCollection()->wellPaths;
            
            QIcon wellIcon(":/Well.png");
            for (size_t i = 0; i < wellPaths.size(); i++)
            {
                options.push_back(caf::PdmOptionItemInfo(wellPaths[i]->name(), QVariant::fromValue(caf::PdmPointer<caf::PdmObjectHandle>(wellPaths[i])), false, wellIcon));
            }
        }

        if (options.size() == 0)
        {
            options.push_front(caf::PdmOptionItemInfo("None", QVariant::fromValue(caf::PdmPointer<caf::PdmObjectHandle>(NULL))));
        }
    }
    else if (fieldNeedingOptions == &simulationWell)
    {
        RimEclipseWellCollection* coll = simulationWellCollection();
        if (coll)
        {
            caf::PdmChildArrayField<RimEclipseWell*>& eclWells = coll->wells;

            QIcon simWellIcon(":/Well.png");
            for (size_t i = 0; i < eclWells.size(); i++)
            {
                options.push_back(caf::PdmOptionItemInfo(eclWells[i]->name(), QVariant::fromValue(caf::PdmPointer<caf::PdmObjectHandle>(eclWells[i])), false, simWellIcon));
            }

        }

        if (options.size() == 0)
        {
            options.push_front(caf::PdmOptionItemInfo("None", QVariant::fromValue(caf::PdmPointer<caf::PdmObjectHandle>(NULL))));
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimCrossSection::userDescriptionField()
{
    return &name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimCrossSection::objectToggleField()
{
    return &isActive;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseWellCollection* RimCrossSection::simulationWellCollection()
{
    RimEclipseView* eclipseView = NULL;
    firstAnchestorOrThisOfType(eclipseView);

    if (eclipseView)
    {
        return eclipseView->wellCollection;
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector< std::vector <cvf::Vec3d> > RimCrossSection::polyLines() const
{
    std::vector< std::vector <cvf::Vec3d> > line;
    if (type == CS_WELL_PATH)
    {
        if (wellPath)
        {
            line.push_back(wellPath->wellPathGeometry()->m_wellPathPoints);
        }
    }
    else if (type == CS_SIMULATION_WELL)
    {

    }
    else
    {

    }

    return line;
}

