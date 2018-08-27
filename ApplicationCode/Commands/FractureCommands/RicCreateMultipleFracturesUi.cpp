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

#include "RicCreateMultipleFracturesUi.h"

#include "RifEclipseDataTableFormatter.h"

#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigWellPath.h"

#include "RimEclipseCase.h"
#include "RimFractureTemplate.h"
#include "RimTools.h"
#include "RimWellPath.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiTableViewEditor.h"
#include "cafPdmUiTextEditor.h"
#include "cafSelectionManagerTools.h"

#include "cvfBoundingBox.h"

CAF_PDM_SOURCE_INIT(RiuCreateMultipleFractionsUi, "RiuCreateMultipleFractionsUi");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuCreateMultipleFractionsUi::RiuCreateMultipleFractionsUi()
{
    CAF_PDM_InitFieldNoDefault(&m_sourceCase, "SourceCase", "Case", "", "", "");

    CAF_PDM_InitField(&m_minDistanceFromWellTd, "MinDistanceFromWellTd", 10.0, "Min Distance From Well TD", "", "", "");
    CAF_PDM_InitField(&m_maxFracturesPerWell, "MaxFracturesPerWell", 10, "Max Fractures Per Well", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_options, "Options", "Options", "", "", "");
    m_options.uiCapability()->setUiEditorTypeName(caf::PdmUiTableViewEditor::uiEditorTypeName());
    m_options.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::TOP);
    m_options.uiCapability()->setCustomContextMenuEnabled(true);

    CAF_PDM_InitFieldNoDefault(&m_fractureCreationSummary, "FractureCreationSummary", "Generated Fractures", "", "", "");
    m_fractureCreationSummary.registerGetMethod(this, &RiuCreateMultipleFractionsUi::summaryText);
    m_fractureCreationSummary.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::TOP);
    m_fractureCreationSummary.uiCapability()->setUiEditorTypeName(caf::PdmUiTextEditor::uiEditorTypeName());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuCreateMultipleFractionsUi::setValues(RimEclipseCase* eclipseCase, double minimumDistanceFromWellToe, int maxFracturesPerWell)
{
    m_sourceCase = eclipseCase;
    m_minDistanceFromWellTd = minimumDistanceFromWellToe;
    m_maxFracturesPerWell = maxFracturesPerWell;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuCreateMultipleFractionsUi::resetValues()
{
    m_sourceCase = nullptr;
    m_options.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RicCreateMultipleFracturesOptionItemUi*> RiuCreateMultipleFractionsUi::options() const
{
    return m_options.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuCreateMultipleFractionsUi::insertOptionItem(RicCreateMultipleFracturesOptionItemUi* insertBeforeThisObject,
                                                    RicCreateMultipleFracturesOptionItemUi* objectToInsert)
{
    size_t index = m_options.index(insertBeforeThisObject);
    if (index < m_options.size())
    {
        m_options.insert(index, objectToInsert);
    }
    else
    {
        m_options.push_back(objectToInsert);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuCreateMultipleFractionsUi::deleteOptionItem(RicCreateMultipleFracturesOptionItemUi* optionsItem)
{
    m_options.removeChildObject(optionsItem);
    delete optionsItem;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuCreateMultipleFractionsUi::clearOptions()
{
    m_options.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuCreateMultipleFractionsUi::addWellPath(RimWellPath* wellPath)
{
    m_wellPaths.push_back(wellPath);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuCreateMultipleFractionsUi::clearWellPaths()
{
    m_wellPaths.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RiuCreateMultipleFractionsUi::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                                  bool*                      useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_sourceCase)
    {
        RimTools::caseOptionItems(&options);
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuCreateMultipleFractionsUi::defineCustomContextMenu(const caf::PdmFieldHandle* fieldNeedingMenu,
                                                           QMenu*                     menu,
                                                           QWidget*                   fieldEditorWidget)
{
    caf::CmdFeatureMenuBuilder menuBuilder;

    menuBuilder << "RicNewOptionItemFeature";
    menuBuilder << "RicDeleteOptionItemFeature";

    menuBuilder.appendToMenu(menu);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuCreateMultipleFractionsUi::summaryText() const
{
    std::vector<LocationForNewFracture> locations = locationsForNewFractures();

    std::set<RimWellPath*>         wellPathSet;
    std::set<RimFractureTemplate*> fracTemplateSet;

    for (auto location : locations)
    {
        wellPathSet.insert(location.wellPath);
        fracTemplateSet.insert(location.fractureTemplate);
    }

    QString tableText;

    {
        QTextStream                  stream(&tableText);
        RifEclipseDataTableFormatter formatter(stream);
        formatter.setTableRowLineAppendText("");
        formatter.setTableRowPrependText("   ");

        std::vector<RifEclipseOutputTableColumn> header;
        header.push_back(RifEclipseOutputTableColumn("Selected Wells"));

        for (auto fracTemplate : fracTemplateSet)
        {
            header.push_back(RifEclipseOutputTableColumn(
                fracTemplate->name(), RifEclipseOutputTableDoubleFormatting(), RifEclipseOutputTableAlignment::RIGHT));
        }

        formatter.header(header);

        for (auto wellPath : wellPathSet)
        {
            formatter.add(wellPath->name());

            for (auto fractureTemplate : fracTemplateSet)
            {
                size_t fractureTemplateCount = 0;
                for (auto fracLocation : locations)
                {
                    if (fractureTemplate == fracLocation.fractureTemplate && wellPath == fracLocation.wellPath)
                    {
                        fractureTemplateCount++;
                    }
                }

                formatter.add(fractureTemplateCount);
            }

            formatter.rowCompleted();
        }

        formatter.tableCompleted();
    }

    return tableText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuCreateMultipleFractionsUi::defineEditorAttribute(const caf::PdmFieldHandle* field,
                                                         QString                    uiConfigName,
                                                         caf::PdmUiEditorAttribute* attribute)
{
    if (field == &m_fractureCreationSummary)
    {
        auto attr = dynamic_cast<caf::PdmUiTextEditorAttribute*>(attribute);
        if (attr)
        {
            QFont font("Courier", 8);

            attr->font = font;
            attr->wrapMode = caf::PdmUiTextEditorAttribute::NoWrap;
        }
    }
    else if (field == &m_options)
    {
        auto attr = dynamic_cast<caf::PdmUiTableViewEditorAttribute*>(attribute);
        if (attr)
        {
            attr->minimumHeight = 130;
            attr->columnWidths = { 90, 90, 400, 70 };
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<LocationForNewFracture> RiuCreateMultipleFractionsUi::locationsForNewFractures() const
{
    std::vector<LocationForNewFracture> items;

    RigMainGrid* mainGrid = nullptr;

    if (m_sourceCase && m_sourceCase->eclipseCaseData())
    {
        mainGrid = m_sourceCase->eclipseCaseData()->mainGrid();
    }

    if (mainGrid)
    {
        //std::vector<RimWellPath*> selWells = caf::selectedObjectsByTypeStrict<RimWellPath*>();

        for (auto w : m_wellPaths)
        {
            auto wellPathGeometry = w->wellPathGeometry();
            if (wellPathGeometry)
            {
                auto mdOfWellPathTip = wellPathGeometry->measureDepths().back();

                int fractureCountForWell = 0;

                for (const auto& option : m_options)
                {
                    double currentMeasuredDepth = mdOfWellPathTip - m_minDistanceFromWellTd;

                    bool continueSearch = true;
                    if (fractureCountForWell >= m_maxFracturesPerWell)
                    {
                        continueSearch = false;
                    }

                    while (continueSearch)
                    {
                        auto locationInDomainCoords = wellPathGeometry->interpolatedPointAlongWellPath(currentMeasuredDepth);

                        size_t reservoirGlobalCellIndex = mainGrid->findReservoirCellIndexFromPoint(locationInDomainCoords);

                        if (reservoirGlobalCellIndex != cvf::UNDEFINED_SIZE_T)
                        {
                            size_t i;
                            size_t j;
                            size_t k;
                            mainGrid->ijkFromCellIndex(reservoirGlobalCellIndex, &i, &j, &k);

                            int oneBasedK = static_cast<int>(k) + 1;
                            if (option->isKLayerContained(oneBasedK))
                            {
                                if (option->fractureTemplate())
                                {
                                    items.push_back(LocationForNewFracture(option->fractureTemplate(), w, currentMeasuredDepth));
                                    fractureCountForWell++;
                                }
                            }
                        }

                        currentMeasuredDepth -= option->minimumSpacing();

                        if (currentMeasuredDepth < 0)
                        {
                            continueSearch = false;
                        }

                        if (fractureCountForWell >= m_maxFracturesPerWell)
                        {
                            continueSearch = false;
                        }
                    }
                }
            }
        }
    }

    return items;
}
