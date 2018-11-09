/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "RimGridCollection.h"
#include "RimEclipseCase.h"
#include "RimGridView.h"

#include "RigMainGrid.h"

#include "cafPdmUiTreeOrdering.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void removeGridInfo(const QString& gridName, std::vector<RimGridInfo*>& collection)
{
    for (size_t i = 0; i < collection.size(); i++)
    {
        if (collection[i]->name() == gridName)
        {
            collection.erase(collection.begin() + i);
            return;
        }
    }
}

CAF_PDM_SOURCE_INIT(RimGridInfo, "GridInfo");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridInfo::RimGridInfo()
{
    CAF_PDM_InitObject("GridInfo", ":/draw_style_meshlines_24x24.png", "", "");

    CAF_PDM_InitField(&m_isActive, "IsActive", true, "Show Grid Cells", "", "", "");
    CAF_PDM_InitField(&m_gridName, "GridName", QString(), "Grid Name", "", "", "");
    m_gridName.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitField(&m_gridIndex, "GridIndex", 0, "Grid Index", "", "", "");
    m_gridIndex.uiCapability()->setUiReadOnly(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimGridInfo::objectToggleField()
{
    return &m_isActive;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridInfo::setName(const QString& name)
{
    m_gridName = name;
    setUiName(name);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridInfo::setIndex(int index)
{
    m_gridIndex = index;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridInfo::setActive(bool active)
{
    m_isActive = active;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridInfo::isActive() const
{
    return m_isActive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridInfo::name() const
{
    return m_gridName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimGridInfo::index() const
{
    return m_gridIndex();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimGridInfo::userDescriptionField()
{
    return &m_gridName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridInfo::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimGridView* rimView = nullptr;
    this->firstAncestorOrThisOfType(rimView);

    rimView->scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridInfo::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_gridName);
    uiOrdering.add(&m_gridIndex);

    uiOrdering.skipRemainingFields();
}

CAF_PDM_SOURCE_INIT(RimGridInfoCollection, "GridInfoCollection");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridInfoCollection::RimGridInfoCollection()
{
    CAF_PDM_InitObject("GridInfoCollection", ":/draw_style_meshlines_24x24.png", "", "");

    CAF_PDM_InitField(&m_isActive, "IsActive", true, "Show Grid Cells", "", "", "");
    m_isActive.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_gridInfos, "GridInfos", "Grid Infos", "", "", "");

    m_gridInfos.uiCapability()->setUiTreeHidden(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridInfoCollection::isActive() const
{
    return m_isActive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridInfoCollection::addGridInfo(const QString& name, size_t gridIndex)
{
    auto gridInfo = new RimGridInfo();
    gridInfo->setName(name);
    gridInfo->setIndex((int)gridIndex);
    m_gridInfos.push_back(gridInfo);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridInfoCollection::clear()
{
    m_gridInfos.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridInfoCollection::containsGrid(const QString& gridName) const
{
    for (auto gridInfo : m_gridInfos)
    {
        if (gridInfo->name() == gridName) return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridInfoCollection::deleteGridInfo(const QString& gridName)
{
    for (size_t i = 0; i < m_gridInfos.size(); i++)
    {
        auto gridInfo = m_gridInfos[i];
        if (gridInfo->name() == gridName)
        {
            m_gridInfos.erase(i);
            delete gridInfo;
            break;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimGridInfo*> RimGridInfoCollection::gridInfos() const
{
    return m_gridInfos.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimGridInfoCollection::objectToggleField()
{
    return &m_isActive;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridInfoCollection::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                             const QVariant&            oldValue,
                                             const QVariant&            newValue)
{
    RimGridView* rimView = nullptr;
    this->firstAncestorOrThisOfType(rimView);

    rimView->scheduleCreateDisplayModelAndRedraw();
}

CAF_PDM_SOURCE_INIT(RimGridCollection, "GridCollection");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCollection::RimGridCollection()
{
    CAF_PDM_InitObject("Grids", ":/draw_style_meshlines_24x24.png", "", "");

    CAF_PDM_InitField(&m_isActive, "IsActive", true, "Show Grid Cells", "", "", "");
    m_isActive.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_mainGrid, "MainGrid", "Main Grid", "", "", "");
    m_mainGrid = new RimGridInfo();
    m_mainGrid->setUiName("Main Grid");
    m_mainGrid->uiCapability()->setUiTreeHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_persistentLgrs, "PersistentLgrs", "Persistent LGRs", "", "", "");
    m_persistentLgrs = new RimGridInfoCollection();
    m_persistentLgrs->setUiName(persistentGridUiName());

    CAF_PDM_InitFieldNoDefault(&m_temporaryLgrs, "TemporaryLgrs", "Temporary LGRs", "", "", "");
    m_temporaryLgrs.xmlCapability()->disableIO();
    m_temporaryLgrs = new RimGridInfoCollection();
    m_temporaryLgrs->setUiName(temporaryGridUiName());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCollection::~RimGridCollection() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString RimGridCollection::persistentGridUiName()
{
    return "LGRs";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString RimGridCollection::temporaryGridUiName()
{
    return "Temporary LGRs";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RimGridCollection::indicesToVisibleGrids() const
{
    std::vector<size_t> gridIndices;

    if (!isActive()) return gridIndices;

    if (m_mainGrid()->isActive())
    {
        gridIndices.push_back(m_mainGrid->index());
    }

    if (m_persistentLgrs()->isActive())
    {
        for (const auto& gridInfo : m_persistentLgrs->gridInfos())
        {
            if (gridInfo->isActive())
            {
                gridIndices.push_back(gridInfo->index());
            }
        }
    }

    if (m_temporaryLgrs()->isActive())
    {
        for (const auto& gridInfo : m_temporaryLgrs->gridInfos())
        {
            if (gridInfo->isActive())
            {
                gridIndices.push_back(gridInfo->index());
            }
        }
    }

    return gridIndices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCollection::setActive(bool active)
{
    m_isActive = active;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridCollection::isActive() const
{
    return m_isActive;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimGridCollection::objectToggleField()
{
    return &m_isActive;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCollection::syncFromMainGrid()
{
    auto mainGrid = this->mainGrid();
    if (mainGrid)
    {
        m_mainGrid->setName("Main Grid");
        m_mainGrid->setIndex(0);

        auto allTemporaryGrids  = m_temporaryLgrs->gridInfos();
        auto allPersistentGrids = m_persistentLgrs->gridInfos();

        size_t gridCount = mainGrid->gridCount();
        for (size_t i = 1; i < gridCount; i++)
        {
            auto    grid      = mainGrid->gridByIndex(i);
            QString gridName  = QString::fromStdString(grid->gridName());
            size_t  gridIndex = grid->gridIndex();

            if (grid->isTempGrid())
            {
                if (m_temporaryLgrs->containsGrid(gridName))
                {
                    removeGridInfo(gridName, allTemporaryGrids);
                }
                else
                {
                    m_temporaryLgrs->addGridInfo(gridName, gridIndex);
                }
            }
            else
            {
                if (m_persistentLgrs->containsGrid(gridName))
                {
                    removeGridInfo(gridName, allPersistentGrids);
                }
                else
                {
                    m_persistentLgrs->addGridInfo(gridName, gridIndex);
                }
            }
        }

        for (const auto& grid : allPersistentGrids)
        {
            auto gridName = grid->name();
            m_persistentLgrs->deleteGridInfo(gridName);
        }

        for (const auto& grid : allTemporaryGrids)
        {
            auto gridName = grid->name();
            m_temporaryLgrs->deleteGridInfo(gridName);
        }
    }
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCollection::setMainGridActive(bool active)
{
    m_mainGrid()->setActive(active);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCollection::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                         const QVariant&            oldValue,
                                         const QVariant&            newValue)
{
    if (changedField == &m_isActive)
    {
        RimGridView* rimView = nullptr;
        this->firstAncestorOrThisOfType(rimView);
        CVF_ASSERT(rimView);

        if (rimView) rimView->showGridCells(m_isActive);

        updateUiIconFromState(m_isActive);
    }

    RimGridView* rimView = nullptr;
    this->firstAncestorOrThisOfType(rimView);

    rimView->scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCollection::initAfterRead()
{
    updateUiIconFromState(m_isActive);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCollection::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    uiTreeOrdering.add(m_mainGrid());
    if (hasPersistentLgrs())
    {
        uiTreeOrdering.add(m_persistentLgrs());
    }
    uiTreeOrdering.add(m_temporaryLgrs());
    uiTreeOrdering.skipRemainingChildren(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigMainGrid* RimGridCollection::mainGrid() const
{
    RimEclipseCase* eclipseCase;
    firstAncestorOrThisOfType(eclipseCase);
    return eclipseCase ? eclipseCase->mainGrid() : nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridCollection::hasPersistentLgrs() const
{
    auto mainGrid = this->mainGrid();
    if (!mainGrid) return 0;

    for (int i = 1; i < mainGrid->gridCount(); i++)
    {
        const auto grid = mainGrid->gridByIndex(i);
        if (!grid->isTempGrid()) return true;
    }
    return false;
}
