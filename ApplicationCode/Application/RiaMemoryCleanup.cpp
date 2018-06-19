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
#include "RigCaseCellResultsData.h"
#include "RigFemPartResultsCollection.h"
#include "RigFemResultAddress.h"
#include "RigGeoMechCaseData.h"
#include "Rim3dView.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultDefinition.h"
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
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case());
    RimGeoMechCase* geoMechCase = dynamic_cast<RimGeoMechCase*>(m_case());
    if (eclipseCase)
    {
        RigCaseCellResultsData* caseData = eclipseCase->results(RiaDefines::MATRIX_MODEL);
        if (caseData)
        {
            std::vector<RigEclipseResultInfo> resultsToDelete = selectedEclipseResults();
            for (const RigEclipseResultInfo& resultInfo : resultsToDelete)
            {
                caseData->clearScalarResult(resultInfo);
            }
        }
    }
    else if (geoMechCase)
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
    m_eclipseResultAddresses.clear();
    m_geomResultAddresses.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigFemResultAddress> RiaMemoryCleanup::selectedGeoMechResults() const
{
    std::vector<RigFemResultAddress> results;
    if (dynamic_cast<const RimGeoMechCase*>(m_case()))
    {
        for (size_t index : m_resultsToDelete())
        {
            CVF_ASSERT(index < m_geomResultAddresses.size());
            results.push_back(m_geomResultAddresses[index]);
        }
    }
    return results;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigEclipseResultInfo> RiaMemoryCleanup::selectedEclipseResults() const
{
    std::vector<RigEclipseResultInfo> results;
    if (dynamic_cast<const RimEclipseCase*>(m_case()))
    {
        for (size_t index : m_resultsToDelete())
        {
            CVF_ASSERT(index < m_eclipseResultAddresses.size());
            results.push_back(m_eclipseResultAddresses[index]);
        }
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
std::set<RigEclipseResultInfo> RiaMemoryCleanup::findEclipseResultsInUse() const
{
    std::set<RigEclipseResultInfo> resultsInUse;
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case());
    if (eclipseCase)
    {
        std::vector<RimEclipseResultDefinition*> eclipseResultDefs;
        eclipseCase->descendantsIncludingThisOfType(eclipseResultDefs);
        for (RimEclipseResultDefinition* resultDef : eclipseResultDefs)
        {
            RigEclipseResultInfo resultInfo(resultDef->resultType(), resultDef->resultVariable());
            resultsInUse.insert(resultInfo);
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
        RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case());
        RimGeoMechCase* geoMechCase = dynamic_cast<RimGeoMechCase*>(m_case());
        if (eclipseCase)
        {
            std::set<RigEclipseResultInfo> resultsInUse = findEclipseResultsInUse();
            RigCaseCellResultsData* caseData = eclipseCase->results(RiaDefines::MATRIX_MODEL);
            if (caseData)
            {
                m_eclipseResultAddresses = caseData->infoForEachResultIndex();

                for (size_t i = 0; i < m_eclipseResultAddresses.size(); ++i)
                {
                    const RigEclipseResultInfo& result = m_eclipseResultAddresses[i];
                    if (caseData->isResultLoaded(result))
                    {
                        bool inUse = resultsInUse.count(result);
                        QString posText = caf::AppEnum<RiaDefines::ResultCatType>::uiTextFromIndex(result.resultType());
                        QString resultsText = QString("%1, %2").arg(posText).arg(result.resultName());
                        if (inUse)
                        {
                            resultsText += QString(" [used in view]");
                        }
                        options.push_back(caf::PdmOptionItemInfo(resultsText, (qulonglong)i, inUse));
                    }
                }
            }
            
        }
        else if (geoMechCase)
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
                        resultsText += QString(" [used in view]");
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
