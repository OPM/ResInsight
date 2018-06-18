/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
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

#include "RiaMemoryCleanup.h"

#include "RiaApplication.h"
#include "RigFemPartResultsCollection.h"
#include "RigFemResultAddress.h"
#include "RigGeoMechCaseData.h"
#include "Rim3dView.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechResultDefinition.h"
#include "RimProject.h"

#include "cafPdmUiListEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTreeSelectionEditor.h"

//==================================================================================================
///
///
//==================================================================================================

CAF_PDM_SOURCE_INIT(RiaMemoryCleanup, "RiaMemoryCleanup");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaMemoryCleanup::RiaMemoryCleanup()
{
    // clang-format off
    CAF_PDM_InitFieldNoDefault(&m_case, "DataCase", "Case", "", "", "");
    m_case = nullptr;

    CAF_PDM_InitFieldNoDefault(&m_resultsToDelete, "ResultsToDelete", "Results In Memory", "", "", "");
    m_resultsToDelete.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::TOP);
    m_resultsToDelete.uiCapability()->setUiEditorTypeName(caf::PdmUiTreeSelectionEditor::uiEditorTypeName());

    CAF_PDM_InitFieldNoDefault(&m_performDelete, "ClearSelectedData", "", "", "", "");
    caf::PdmUiPushButtonEditor::configureEditorForField(&m_performDelete);
    // clang-format on
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaMemoryCleanup::setPropertiesFromView(Rim3dView* view)
{
    if (!view) return;

    m_case = view->ownerCase();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaMemoryCleanup::clearSelectedResultsFromMemory()
{
    RimGeoMechCase* geoMechCase = dynamic_cast<RimGeoMechCase*>(m_case());
    if (geoMechCase)
    {
        RigGeoMechCaseData* data = geoMechCase->geoMechData();
        if (data)
        {
            RigFemPartResultsCollection* resultsCollection = data->femPartResults();
            if (resultsCollection)
            {
                std::vector<RigFemResultAddress> resultsToDelete = selectedGeoMechResults();
                for (RigFemResultAddress result : resultsToDelete)
                {
                    resultsCollection->deleteResult(result);                    
                }
            }
        }
    }
    m_resultsToDelete.v().clear();
    m_geomResultAddresses.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigFemResultAddress> RiaMemoryCleanup::selectedGeoMechResults() const
{
    std::vector<RigFemResultAddress> results;
    for (size_t index : m_resultsToDelete())
    {
        results.push_back(m_geomResultAddresses[index]);
    }
    return results;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RigFemResultAddress> RiaMemoryCleanup::findGeoMechCaseResultsInUse() const
{
    std::set<RigFemResultAddress> resultsInUse;
    RimGeoMechCase*               geoMechCase = dynamic_cast<RimGeoMechCase*>(m_case());
    if (geoMechCase)
    {
        std::vector<RimFemResultObserver*> geoMechResults;
        geoMechCase->descendantsIncludingThisOfType(geoMechResults);
        for (RimFemResultObserver* resultDef : geoMechResults)
        {
            caf::PdmField<bool>* field = dynamic_cast<caf::PdmField<bool>*>(resultDef->objectToggleField());
            if (!field || (*field)())
            {
                std::vector<RigFemResultAddress> required = resultDef->observedResults();
                resultsInUse.insert(required.begin(), required.end());
            }
        }
    }
    return resultsInUse;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaMemoryCleanup::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                        const QVariant&            oldValue,
                                        const QVariant&            newValue)
{
    if (changedField == &m_case)
    {
        m_resultsToDelete.uiCapability()->updateConnectedEditors();
    }
    else if (changedField == &m_performDelete)
    {
        clearSelectedResultsFromMemory();
        m_resultsToDelete.uiCapability()->updateConnectedEditors();
        m_performDelete = false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RiaMemoryCleanup::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                      bool*                      useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;
    if (fieldNeedingOptions == &m_case)
    {
        RimProject* proj = RiaApplication::instance()->project();
        if (proj)
        {
            std::vector<RimGeoMechCase*> cases = proj->geoMechCases();

            for (RimGeoMechCase* c : cases)
            {
                options.push_back(caf::PdmOptionItemInfo(c->caseUserDescription(), c, false, c->uiIcon()));
            }
        }
    }
    else if (fieldNeedingOptions == &m_resultsToDelete)
    {
        RimGeoMechCase* geoMechCase = dynamic_cast<RimGeoMechCase*>(m_case());
        if (geoMechCase)
        {
            std::set<RigFemResultAddress> resultsInUse = findGeoMechCaseResultsInUse();
            RigGeoMechCaseData*           caseData     = geoMechCase->geoMechData();
            if (caseData)
            {
                RigFemPartResultsCollection* results = caseData->femPartResults();
                m_geomResultAddresses                = results->loadedResults();

                for (size_t i = 0; i < m_geomResultAddresses.size(); ++i)
                {
                    const RigFemResultAddress& result  = m_geomResultAddresses[i];
                    bool                       inUse   = resultsInUse.count(result);
                    QString                    posText = caf::AppEnum<RigFemResultPosEnum>::uiTextFromIndex(result.resultPosType);
                    QString resultsText = QString("%1, %2").arg(posText).arg(QString::fromStdString(result.fieldName));
                    if (!result.componentName.empty())
                    {
                        resultsText += QString(", %1").arg(QString::fromStdString(result.componentName));
                    }
                    if (inUse)
                    {
                        resultsText += QString(" [shown in view]");
                    }
                    options.push_back(caf::PdmOptionItemInfo(resultsText, (qulonglong) i, inUse));
                }
            }
        }
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaMemoryCleanup::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_case);
    uiOrdering.add(&m_resultsToDelete);
    uiOrdering.add(&m_performDelete);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaMemoryCleanup::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    if (field == &m_performDelete)
    {
        caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>(attribute);
        if (attrib)
        {
            attrib->m_buttonText = "Clear Checked Data From Memory";
        }
    }
}
