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

#include "RimPerforationCollection.h"

#include "RiaApplication.h"

#include "Rim3dView.h"
#include "RimEclipseCase.h"
#include "RimPerforationInterval.h"
#include "RimProject.h"
#include "RimMswCompletionParameters.h"
#include "RimNonDarcyPerforationParameters.h"

#include "RigWellPath.h"

#include "RifWellPathImporter.h"

#include "Riu3DMainWindowTools.h"


CAF_PDM_SOURCE_INIT(RimPerforationCollection, "PerforationCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimPerforationCollection::RimPerforationCollection()
{
    CAF_PDM_InitObject("Perforations", ":/PerforationIntervals16x16.png", "", "");

    nameField()->uiCapability()->setUiHidden(true);
    this->setName("Perforations");

    CAF_PDM_InitFieldNoDefault(&m_perforations, "Perforations", "Perforations", "", "", "");
    m_perforations.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_mswParameters, "MswParameters", "Multi Segment Well Parameters", "", "", "");
    m_mswParameters = new RimMswCompletionParameters;
    m_mswParameters.uiCapability()->setUiTreeHidden(true);
    m_mswParameters.uiCapability()->setUiTreeChildrenHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_nonDarcyParameters, "NonDarcyParameters", "Non-Darcy Parameters", "", "", "");
    m_nonDarcyParameters = new RimNonDarcyPerforationParameters();
    m_nonDarcyParameters.uiCapability()->setUiTreeHidden(true);
    m_nonDarcyParameters.uiCapability()->setUiTreeChildrenHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimPerforationCollection::~RimPerforationCollection()
{
    m_perforations.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RimMswCompletionParameters* RimPerforationCollection::mswParameters() const
{
    return m_mswParameters;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RimNonDarcyPerforationParameters* RimPerforationCollection::nonDarcyParameters() const
{
    return m_nonDarcyParameters;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPerforationCollection::setUnitSystemSpecificDefaults()
{
    m_mswParameters->setUnitSystemSpecificDefaults();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPerforationCollection::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* mswGroup = uiOrdering.addNewGroup("Multi Segment Well Options");
    m_mswParameters->uiOrdering(uiConfigName, *mswGroup);

    m_nonDarcyParameters->uiOrdering(uiConfigName, uiOrdering);
    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPerforationCollection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimProject* proj;
    this->firstAncestorOrThisOfTypeAsserted(proj);
    if (changedField == &m_isChecked)
    {
        proj->reloadCompletionTypeResultsInAllViews();
    }
    else
    {
        proj->scheduleCreateDisplayModelAndRedrawAllViews();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPerforationCollection::appendPerforation(RimPerforationInterval* perforation)
{
    QDate firstTimeStepFromCase;
    QDate lastTimeStepFromCase;
    Rim3dView* activeView = RiaApplication::instance()->activeReservoirView();
    if (activeView)
    {
        activeView->hasUserRequestedAnimation = true;

        RimEclipseCase* eclipseCase = nullptr;
        activeView->firstAncestorOrThisOfType(eclipseCase);
        if (eclipseCase)
        {
            auto dates = eclipseCase->timeStepDates();
            if (!dates.empty())
            {
                firstTimeStepFromCase = dates.front().date();

                lastTimeStepFromCase = dates.back().date();
            }
        }
    }

    if (firstTimeStepFromCase.isValid())
    {
        perforation->setCustomStartDate(firstTimeStepFromCase);
    }

    if (lastTimeStepFromCase.isValid())
    {
        perforation->setCustomEndDate(lastTimeStepFromCase);
    }

    m_perforations.push_back(perforation);

    perforation->setUnitSystemSpecificDefaults();

    updateConnectedEditors();
    Riu3DMainWindowTools::selectAsCurrentItem(perforation);

    RimProject* proj;
    this->firstAncestorOrThisOfTypeAsserted(proj);
    proj->reloadCompletionTypeResultsInAllViews();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<const RimPerforationInterval*> RimPerforationCollection::perforations() const
{
    std::vector<const RimPerforationInterval*> myPerforations;

    for (const auto& perforation : m_perforations)
    {
        myPerforations.push_back(perforation);
    }

    return myPerforations;
}

