/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RimMultiSnapshotDefinition.h"

#include "RiaApplication.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigReservoirGridTools.h"

#include "RimCase.h"
#include "RimEclipseView.h"
#include "RimProject.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimTools.h"
#include "RimView.h"

#include "cafPdmPointer.h"

namespace caf
{
    template<>
    void caf::AppEnum< RimMultiSnapshotDefinition::SnapShotDirectionEnum >::setUp()
    {
        addItem(RimMultiSnapshotDefinition::NO_RANGEFILTER, "None", "None");
        addItem(RimMultiSnapshotDefinition::RANGEFILTER_I, "I", "I");
        addItem(RimMultiSnapshotDefinition::RANGEFILTER_J, "J", "J");
        addItem(RimMultiSnapshotDefinition::RANGEFILTER_K, "K", "K");

        setDefault(RimMultiSnapshotDefinition::RANGEFILTER_K);
    }
}

CAF_PDM_SOURCE_INIT(RimMultiSnapshotDefinition, "MultiSnapshotDefinition");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimMultiSnapshotDefinition::RimMultiSnapshotDefinition()
{
    //CAF_PDM_InitObject("MultiSnapshotDefinition", ":/Well.png", "", "");
    CAF_PDM_InitObject("MultiSnapshotDefinition", "", "", "");

    CAF_PDM_InitField(&isActive,                        "IsActive", true,           "Active", "", "", "");
    
    CAF_PDM_InitFieldNoDefault(&view,                   "View",                     "View", "", "", "");

    CAF_PDM_InitFieldNoDefault(&eclipseResultType,      "EclipseResultType",        "Result Type", "", "", "");
    CAF_PDM_InitFieldNoDefault(&selectedEclipseResults, "SelectedEclipseResults",   "Properties", "", "", "");

    CAF_PDM_InitField(&timeStepStart,                   "TimeStepStart", 0,         "Start Time", "", "", "");
    CAF_PDM_InitField(&timeStepEnd,                     "TimeStepEnd", 0,           "End Time", "", "", "");

    CAF_PDM_InitField(&sliceDirection,                  "SnapShotDirection",    caf::AppEnum<SnapShotDirectionEnum>(NO_RANGEFILTER), "Range Filter Slice", "", "", "");
    CAF_PDM_InitField(&startSliceIndex,                 "RangeFilterStart", 1,      "Range Start", "", "", "");
    CAF_PDM_InitField(&endSliceIndex,                   "RangeFilterEnd", 1,        "Range End", "", "", "");

    CAF_PDM_InitFieldNoDefault(&additionalCases,        "AdditionalCases",          "Cases", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimMultiSnapshotDefinition::~RimMultiSnapshotDefinition()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimMultiSnapshotDefinition::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &view)
    {
        options.push_back(caf::PdmOptionItemInfo("None", nullptr));

        std::vector<RimView*> views; 

        RimProject* proj = RiaApplication::instance()->project();
        std::vector<RimCase*> cases;
        proj->allCases(cases);

        for (RimCase* rimCase : cases)
        {
            for (RimView* rimView : rimCase->views())
            {
                views.push_back(rimView);
            }
        }

        for (RimView* view : views)
        {
            QString caseAndView = view->ownerCase()->caseUserDescription() + " - " + view->name();
            options.push_back(caf::PdmOptionItemInfo(caseAndView, view));
        }
    }
    else if (fieldNeedingOptions == &eclipseResultType)
    {
        options.push_back(caf::PdmOptionItemInfo(caf::AppEnum<RiaDefines::ResultCatType>(RiaDefines::DYNAMIC_NATIVE).uiText(), RiaDefines::DYNAMIC_NATIVE));
        options.push_back(caf::PdmOptionItemInfo(caf::AppEnum<RiaDefines::ResultCatType>(RiaDefines::STATIC_NATIVE).uiText(), RiaDefines::STATIC_NATIVE));
    }
    else if (fieldNeedingOptions == &selectedEclipseResults)
    {
        RimEclipseView* rimEclipseView = dynamic_cast<RimEclipseView*>(view().p());
        if (rimEclipseView)
        {
            QStringList varList;
            varList = rimEclipseView->currentGridCellResults()->cellResults()->resultNames(eclipseResultType());

            options = toOptionList(varList);
        }
    }
    else if (fieldNeedingOptions == &timeStepEnd)
    {
        getTimeStepStrings(options);
    }
    else if (fieldNeedingOptions == &timeStepStart)
    {
        getTimeStepStrings(options);
    }
    else if (fieldNeedingOptions == &additionalCases)
    {
        RimTools::caseOptionItems(&options);
    }

    if (useOptionsOnly) *useOptionsOnly = true;

    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimMultiSnapshotDefinition::getTimeStepStrings(QList<caf::PdmOptionItemInfo> &options)
{
    if (!view()) return;

    QStringList timeSteps;

    timeSteps = view->ownerCase()->timeStepStrings();

    for (int i = 0; i < timeSteps.size(); i++)
    {
        options.push_back(caf::PdmOptionItemInfo(timeSteps[i], i));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimMultiSnapshotDefinition::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &eclipseResultType)
    {
        selectedEclipseResults.v().clear();
    }
    else if (changedField == &sliceDirection)
    {
        const cvf::StructGridInterface* mainGrid = nullptr;
        RigActiveCellInfo* actCellInfo = nullptr;
       
        if (view())
        {
            actCellInfo = RigReservoirGridTools::activeCellInfo(view());

            RimCase* rimCase = nullptr;
            view()->firstAncestorOrThisOfTypeAsserted(rimCase);
            
            mainGrid = RigReservoirGridTools::mainGrid(rimCase);
        }

        if (mainGrid && actCellInfo)
        {
            cvf::Vec3st min, max;
            actCellInfo->IJKBoundingBox(min, max);

            // Adjust to Eclipse indexing
            min.x() = min.x() + 1;
            min.y() = min.y() + 1;
            min.z() = min.z() + 1;

            max.x() = max.x() + 1;
            max.y() = max.y() + 1;
            max.z() = max.z() + 1;

            int maxInt = 0;
            int minInt = 0;

            if (newValue == RimMultiSnapshotDefinition::RANGEFILTER_I)
            {
                maxInt = static_cast<int>(max.x());
                minInt = static_cast<int>(min.x());
            }
            else if (newValue == RimMultiSnapshotDefinition::RANGEFILTER_J)
            {
                maxInt = static_cast<int>(max.y());
                minInt = static_cast<int>(min.y());
            }
            else if (newValue == RimMultiSnapshotDefinition::RANGEFILTER_K)
            {
                maxInt = static_cast<int>(max.z());
                minInt = static_cast<int>(min.z());
            }
                
            startSliceIndex = minInt;
            endSliceIndex = maxInt;

        }
       
        startSliceIndex.uiCapability()->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimMultiSnapshotDefinition::toOptionList(const QStringList& varList)
{
    QList<caf::PdmOptionItemInfo> optionList;
    int i;
    for (i = 0; i < varList.size(); ++i)
    {
        optionList.push_back(caf::PdmOptionItemInfo(varList[i], varList[i]));
    }
    return optionList;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimMultiSnapshotDefinition::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    if (!isActive())
    {
        view.uiCapability()->setUiReadOnly(true);
        eclipseResultType.uiCapability()->setUiReadOnly(true);
        selectedEclipseResults.uiCapability()->setUiReadOnly(true);
        timeStepStart.uiCapability()->setUiReadOnly(true);
        timeStepEnd.uiCapability()->setUiReadOnly(true);
        sliceDirection.uiCapability()->setUiReadOnly(true);
        startSliceIndex.uiCapability()->setUiReadOnly(true);
        endSliceIndex.uiCapability()->setUiReadOnly(true);
        additionalCases.uiCapability()->setUiReadOnly(true);
    }
    else
    {
        view.uiCapability()->setUiReadOnly(false);

        if (!view())
        {
            eclipseResultType.uiCapability()->setUiReadOnly(true);
            selectedEclipseResults.uiCapability()->setUiReadOnly(true);
            timeStepStart.uiCapability()->setUiReadOnly(true);
            timeStepEnd.uiCapability()->setUiReadOnly(true);
            sliceDirection.uiCapability()->setUiReadOnly(true);
            startSliceIndex.uiCapability()->setUiReadOnly(true);
            endSliceIndex.uiCapability()->setUiReadOnly(true);
            additionalCases.uiCapability()->setUiReadOnly(true);
        }
        else
        {
            eclipseResultType.uiCapability()->setUiReadOnly(false);
            selectedEclipseResults.uiCapability()->setUiReadOnly(false);
            timeStepStart.uiCapability()->setUiReadOnly(false);
            timeStepEnd.uiCapability()->setUiReadOnly(false);
            sliceDirection.uiCapability()->setUiReadOnly(false);

            additionalCases.uiCapability()->setUiReadOnly(false);

            bool rangeReadOnly = false;
            if (sliceDirection() == NO_RANGEFILTER)
            {
                rangeReadOnly = true;
            }

            startSliceIndex.uiCapability()->setUiReadOnly(rangeReadOnly);
            endSliceIndex.uiCapability()->setUiReadOnly(rangeReadOnly);
        }
    }
}
