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
#include "RimGridView.h"
#include "RimEclipseCase.h"

#include "RigMainGrid.h"

#include <cafPdmUiTreeOrdering.h>

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
    CAF_PDM_InitField(&m_gridIndex, "GridIndex", 0, "Grid Index", "", "", "");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimGridInfo::objectToggleField()
{
    return nullptr; // &m_isActive;
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
void RimGridInfo::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                   const QVariant& oldValue,
                                   const QVariant& newValue)
{

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
void RimGridInfoCollection::addGridInfo(const QString& name, size_t gridIndex)
{
    auto gridInfo = new RimGridInfo();
    gridInfo->setActive(true);
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
    return nullptr;// &m_isActive;
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridInfoCollection::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                             const QVariant&            oldValue,
                                             const QVariant&            newValue)
{
}

CAF_PDM_SOURCE_INIT(RimGridCollection, "GridCollection");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString RimGridCollection::PERSISTENT_LGR_UI_NAME = "Persistent LGRs";
const QString RimGridCollection::TEMPORARY_LGR_UI_NAME = "Temporary LGRs";

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
    m_persistentLgrs->setUiName(PERSISTENT_LGR_UI_NAME);

    CAF_PDM_InitFieldNoDefault(&m_temporaryLgrs, "TemporaryLgrs", "Temporary LGRs", "", "", "");
    m_temporaryLgrs.xmlCapability()->disableIO();
    m_temporaryLgrs = new RimGridInfoCollection();
    m_temporaryLgrs->setUiName(TEMPORARY_LGR_UI_NAME);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGridCollection::~RimGridCollection()
{

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

        auto allTemporaryGrids = m_temporaryLgrs->gridInfos();
        auto allPersistentGrids = m_persistentLgrs->gridInfos();

        size_t gridCount = mainGrid->gridCount();
        for (size_t i = 1; i < gridCount; i++)
        {
            auto grid = mainGrid->gridByIndex(i);
            QString gridName = QString::fromStdString(grid->gridName());
            size_t gridIndex = grid->gridIndex();

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

        while (!allPersistentGrids.empty())
        {
            auto gridName = allPersistentGrids.front()->name();
            m_persistentLgrs->deleteGridInfo(gridName);
            removeGridInfo(gridName, allPersistentGrids);
        }
        while (!allTemporaryGrids.empty())
        {
            auto gridName = allTemporaryGrids.front()->name();
            m_temporaryLgrs->deleteGridInfo(allTemporaryGrids.front()->name());
            removeGridInfo(gridName, allTemporaryGrids);
        }
    }
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGridCollection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_isActive)
    {
        RimGridView* rimView = nullptr;
        this->firstAncestorOrThisOfType(rimView);
        CVF_ASSERT(rimView);

        if (rimView) rimView->showGridCells(m_isActive);

        updateUiIconFromState(m_isActive);
    }
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
    uiTreeOrdering.add(m_persistentLgrs());
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
