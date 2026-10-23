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
#include "RimEclipseView.h"
#include "RimGridView.h"

#include "RigLocalGrid.h"
#include "RigMainGrid.h"
#include "RigNestedHybridGridReconstructor.h"

#include "cafPdmUiTreeOrdering.h"

#include <set>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void removeGridInfo( const QString& gridName, std::vector<RimGridInfo*>& collection )
{
    for ( size_t i = 0; i < collection.size(); i++ )
    {
        if ( collection[i]->name() == gridName )
        {
            collection.erase( collection.begin() + i );
            return;
        }
    }
}

CAF_PDM_SOURCE_INIT( RimGridInfo, "GridInfo" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridInfo::RimGridInfo()
{
    CAF_PDM_InitObject( "GridInfo", ":/draw_style_meshlines_24x24.png" );

    CAF_PDM_InitField( &m_isActive, "IsActive", true, "Show Grid Cells" );
    CAF_PDM_InitField( &m_gridName, "GridName", QString(), "Grid Name" );
    m_gridName.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_eclipseGridIndex, "GridIndex", 0, "Grid Index" );
    m_eclipseGridIndex.uiCapability()->setUiReadOnly( true );
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
void RimGridInfo::setName( const QString& name )
{
    m_gridName = name;
    setUiName( name );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridInfo::setEclipseGridIndex( int index )
{
    m_eclipseGridIndex = index;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridInfo::resetSimulatorGridIndices( int index )
{
    m_eclipseGridIndex = index;
    m_additionalGridIndices.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridInfo::addSimulatorGridIndex( int index )
{
    m_additionalGridIndices.push_back( index );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<int> RimGridInfo::simulatorGridIndices() const
{
    std::vector<int> indices = { m_eclipseGridIndex() };
    indices.insert( indices.end(), m_additionalGridIndices.begin(), m_additionalGridIndices.end() );
    return indices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridInfo::setActive( bool active )
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
int RimGridInfo::eclipseGridIndex() const
{
    return m_eclipseGridIndex();
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
void RimGridInfo::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    auto rimView = firstAncestorOfType<RimGridView>();

    rimView->scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridInfo::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_gridName );
    uiOrdering.add( &m_eclipseGridIndex );

    uiOrdering.skipRemainingFields();
}

CAF_PDM_SOURCE_INIT( RimGridInfoCollection, "GridInfoCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridInfoCollection::RimGridInfoCollection()
{
    CAF_PDM_InitObject( "GridInfoCollection", ":/draw_style_meshlines_24x24.png" );

    CAF_PDM_InitField( &m_isActive, "IsActive", true, "Show Grid Cells" );
    m_isActive.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_items, "GridInfos", "Grid Infos" );
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
void RimGridInfoCollection::addGridInfo( RimGridInfo* gridInfo )
{
    addItem( gridInfo );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridInfoCollection::clear()
{
    deleteAllItems();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridInfoCollection::containsGrid( const QString& gridName ) const
{
    for ( auto gridInfo : m_items )
    {
        if ( gridInfo->name() == gridName ) return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridInfoCollection::deleteGridInfo( const QString& gridName )
{
    for ( auto* gridInfo : items() )
    {
        if ( gridInfo->name() == gridName )
        {
            deleteItem( gridInfo );
            break;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimGridInfo*> RimGridInfoCollection::gridInfos() const
{
    return items();
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
void RimGridInfoCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    auto rimView = firstAncestorOfType<RimGridView>();

    rimView->scheduleCreateDisplayModelAndRedraw();
}

CAF_PDM_SOURCE_INIT( RimGridCollection, "GridCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCollection::RimGridCollection()
{
    CAF_PDM_InitObject( "Grids", ":/draw_style_meshlines_24x24.png" );

    CAF_PDM_InitField( &m_isActive, "IsActive", true, "Show Grid Cells" );
    m_isActive.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_mainGrid, "MainGrid", "Main Grid" );
    m_mainGrid = new RimGridInfo();
    m_mainGrid->setUiName( "Main Grid" );
    m_mainGrid->uiCapability()->setUiTreeHidden( true );
    m_mainGrid->setUiIconFromResourceString( ":/MainGrid16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_persistentLgrs, "PersistentLgrs", "Persistent LGRs" );
    m_persistentLgrs = new RimGridInfoCollection();
    m_persistentLgrs->setUiName( persistentGridUiName() );
    m_persistentLgrs->setUiIconFromResourceString( ":/LGR16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_temporaryLgrs, "TemporaryLgrs", "Temporary LGRs" );
    m_temporaryLgrs.xmlCapability()->disableIO();
    m_temporaryLgrs = new RimGridInfoCollection();
    m_temporaryLgrs->setUiName( temporaryGridUiName() );
    m_temporaryLgrs->setUiIconFromResourceString( ":/TempLGR16x16.png" );
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

    if ( !isActive() ) return gridIndices;

    // Guard against stale grid infos restored from a project file whose grid set no longer matches
    // the current grid (e.g. regenerated nested-hybrid LGRs): skip indices outside the current range.
    const RigMainGrid* rigMainGrid = mainEclipseGrid();
    const size_t       gridCount   = rigMainGrid ? rigMainGrid->gridCount() : 0;
    auto               addIfValid  = [&]( size_t gridIndex )
    {
        if ( gridIndex < gridCount ) gridIndices.push_back( gridIndex );
    };

    if ( m_mainGrid()->isActive() )
    {
        addIfValid( m_mainGrid->eclipseGridIndex() );
    }

    if ( m_persistentLgrs()->isActive() )
    {
        for ( const auto& gridInfo : m_persistentLgrs->gridInfos() )
        {
            if ( gridInfo->isActive() )
            {
                for ( int index : gridInfo->simulatorGridIndices() )
                    addIfValid( index );
            }
        }
    }

    if ( m_temporaryLgrs()->isActive() )
    {
        for ( const auto& gridInfo : m_temporaryLgrs->gridInfos() )
        {
            if ( gridInfo->isActive() )
            {
                addIfValid( gridInfo->eclipseGridIndex() );
            }
        }
    }

    return gridIndices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCollection::setActive( bool active )
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
void RimGridCollection::syncFromMainEclipseGrid()
{
    auto mainGrid = mainEclipseGrid();
    if ( mainGrid )
    {
        m_mainGrid->setName( "Main Grid" );
        m_mainGrid->setEclipseGridIndex( 0 );

        auto allTemporaryGrids  = m_temporaryLgrs->gridInfos();
        auto allPersistentGrids = m_persistentLgrs->gridInfos();

        std::set<QString> groupedInfos;

        size_t gridCount = mainGrid->gridCount();
        for ( size_t i = 1; i < gridCount; i++ )
        {
            auto    grid      = mainGrid->gridByIndex( i );
            QString gridName  = QString::fromStdString( grid->gridName() );
            size_t  gridIndex = grid->gridIndex();

            if ( grid->isTempGrid() )
            {
                if ( m_temporaryLgrs->containsGrid( gridName ) )
                {
                    removeGridInfo( gridName, allTemporaryGrids );
                }
                else
                {
                    auto gridInfo = new RimGridInfo();
                    gridInfo->setName( gridName );
                    gridInfo->setEclipseGridIndex( (int)gridIndex );
                    gridInfo->setUiIconFromResourceString( ":/TempLGR16x16.png" );
                    m_temporaryLgrs->addGridInfo( gridInfo );
                }
            }
            else
            {
                // Reconstructed nested hybrid LGRs are grouped into one entry per refinement level
                const auto*   localGrid = dynamic_cast<const RigLocalGrid*>( grid );
                const bool    isGrouped = localGrid && localGrid->isReconstructedGrid();
                const QString infoName  = isGrouped ? RigNestedHybridGridReconstructor::levelGroupName( gridName ) : gridName;

                RimGridInfo* existing = nullptr;
                for ( auto* info : m_persistentLgrs->gridInfos() )
                    if ( info->name() == infoName ) existing = info;

                if ( existing )
                {
                    removeGridInfo( infoName, allPersistentGrids );
                    if ( isGrouped )
                    {
                        if ( groupedInfos.count( infoName ) )
                            existing->addSimulatorGridIndex( (int)gridIndex );
                        else
                            existing->resetSimulatorGridIndices( (int)gridIndex );
                    }
                }
                else
                {
                    existing = new RimGridInfo();
                    existing->setName( infoName );
                    existing->setEclipseGridIndex( (int)gridIndex );
                    existing->setUiIconFromResourceString( ":/LGR16x16.png" );
                    m_persistentLgrs->addGridInfo( existing );
                }
                if ( isGrouped ) groupedInfos.insert( infoName );
            }
        }

        for ( const auto& grid : allPersistentGrids )
        {
            auto gridName = grid->name();
            m_persistentLgrs->deleteGridInfo( gridName );
        }

        for ( const auto& grid : allTemporaryGrids )
        {
            auto gridName = grid->name();
            m_temporaryLgrs->deleteGridInfo( gridName );
        }
    }
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCollection::setMainGridActive( bool active )
{
    m_mainGrid()->setActive( active );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    auto rimView = firstAncestorOrThisOfTypeAsserted<RimGridView>();

    if ( changedField == &m_isActive )
    {
        if ( rimView ) rimView->showGridCells( m_isActive );

        updateUiIconFromState( m_isActive );
    }

    rimView->scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCollection::initAfterRead()
{
    updateUiIconFromState( m_isActive );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCollection::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    if ( mainEclipseGrid() )
    {
        uiTreeOrdering.add( m_mainGrid() );
        if ( hasPersistentLgrs() )
        {
            uiTreeOrdering.add( m_persistentLgrs() );
        }
        if ( hasTemporaryLgrs() )
        {
            uiTreeOrdering.add( m_temporaryLgrs() );
        }
    }
    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigMainGrid* RimGridCollection::mainEclipseGrid() const
{
    RimEclipseView* gridView = firstAncestorOrThisOfType<RimEclipseView>();
    if ( !gridView ) return nullptr;

    auto eclipseCase = gridView->eclipseCase();
    return eclipseCase ? eclipseCase->mainGrid() : nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridCollection::hasPersistentLgrs() const
{
    auto mainGrid = mainEclipseGrid();
    if ( !mainGrid ) return false;

    for ( size_t i = 1; i < mainGrid->gridCount(); i++ )
    {
        const auto grid = mainGrid->gridByIndex( i );
        if ( !grid->isTempGrid() ) return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridCollection::hasTemporaryLgrs() const
{
    auto mainGrid = mainEclipseGrid();
    if ( !mainGrid ) return false;

    for ( size_t i = 1; i < mainGrid->gridCount(); i++ )
    {
        const auto grid = mainGrid->gridByIndex( i );
        if ( grid->isTempGrid() ) return true;
    }
    return false;
}
