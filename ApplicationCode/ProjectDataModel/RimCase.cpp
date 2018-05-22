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

#include "RimCase.h"

#include "RiaApplication.h"
#include "RimFormationNames.h"
#include "RimFormationNamesCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimTimeStepFilter.h"

#include "cafPdmObjectFactory.h"

#include "Rim2dIntersectionView.h"
#include "Rim2dIntersectionViewCollection.h"
#include "RimIntersection.h"
#include "RimGridView.h"


CAF_PDM_XML_ABSTRACT_SOURCE_INIT(RimCase, "RimCase");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCase::RimCase() : m_isInActiveDestruction(false)
{
    CAF_PDM_InitField(&caseUserDescription, "CaseUserDescription",  QString(), "Case Name", "", "" ,"");

    CAF_PDM_InitField(&caseId, "CaseId", -1, "Case ID", "", "" ,"");
    caseId.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitFieldNoDefault(&activeFormationNames, "DefaultFormationNames", "Formation Names File", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_timeStepFilter, "TimeStepFilter", "Time Step Filter", "", "", "");
    m_timeStepFilter.uiCapability()->setUiHidden(true);
    m_timeStepFilter.uiCapability()->setUiTreeChildrenHidden(true);
    m_timeStepFilter = new RimTimeStepFilter;

    CAF_PDM_InitFieldNoDefault(&m_2dIntersectionViewCollection, "IntersectionViewCollection", "2D Intersection Views", ":/CrossSections16x16.png", "", "");
    m_2dIntersectionViewCollection.uiCapability()->setUiTreeHidden(true);
    m_2dIntersectionViewCollection = new Rim2dIntersectionViewCollection();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCase::~RimCase()
{
    m_isInActiveDestruction = true; // Needed because destruction of m_intersectionViews results in call to views()
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<Rim3dView*> RimCase::views() const
{
    if (m_isInActiveDestruction) return std::vector<Rim3dView*>();

    std::vector<Rim3dView*> allViews = this->allSpecialViews();
    std::vector<Rim2dIntersectionView*> isectViews = m_2dIntersectionViewCollection->views();

    for (auto view: isectViews)
    {
        allViews.push_back(view);
    }

    return allViews;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimGridView*> RimCase::gridViews() const
{
    std::vector<RimGridView*> grViews;

    for (Rim3dView* const view : views())
    {
        RimGridView* grView = dynamic_cast<RimGridView*>(view);
        if (grView) grViews.push_back(grView);
    }
    return grViews;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimCase::displayModelOffset() const
{
    return cvf::Vec3d::ZERO;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCase::setFormationNames(RimFormationNames* formationNames)
{
    activeFormationNames = formationNames;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RimCase::uiToNativeTimeStepIndex(size_t uiTimeStepIndex)
{
    std::vector<size_t> nativeTimeIndices = m_timeStepFilter->filteredNativeTimeStepIndices();

    if (nativeTimeIndices.size() > 0)
    {
        return nativeTimeIndices.at(uiTimeStepIndex);
    }

    return uiTimeStepIndex;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Rim2dIntersectionViewCollection* RimCase::intersectionViewCollection()
{
    return m_2dIntersectionViewCollection;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimCase::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if(fieldNeedingOptions == &activeFormationNames)
    {
        RimProject* proj = RiaApplication::instance()->project();
        if (proj && proj->activeOilField() && proj->activeOilField()->formationNamesCollection())
        {
            for(RimFormationNames* fnames : proj->activeOilField()->formationNamesCollection()->formationNamesList())
            {
                options.push_back(caf::PdmOptionItemInfo(fnames->fileNameWoPath(), fnames, false, fnames->uiCapability()->uiIcon()));
            }
        }

        options.push_front(caf::PdmOptionItemInfo("None", nullptr));
    }

    return options;
}
