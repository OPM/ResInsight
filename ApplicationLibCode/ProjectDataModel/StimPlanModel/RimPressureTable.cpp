/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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
#include "RimPressureTable.h"

#include "RiaStimPlanModelDefines.h"
#include "RimPressureTableItem.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiTableViewEditor.h"
#include "cafPdmUiTreeOrdering.h"

CAF_PDM_SOURCE_INIT( RimPressureTable, "PressureTable" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPressureTable::RimPressureTable()
    : changed( this )
{
    CAF_PDM_InitObject( "Pressure Table", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_pressureTableItems, "Items", "Pressure Table Items", "", "", "" );
    m_pressureTableItems.uiCapability()->setUiEditorTypeName( caf::PdmUiTableViewEditor::uiEditorTypeName() );
    m_pressureTableItems.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_pressureTableItems.uiCapability()->setCustomContextMenuEnabled( true );

    CAF_PDM_InitFieldNoDefault( &m_pressureDate, "PressureDate", "Pressure Date", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPressureTable::~RimPressureTable()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPressureTableItem*> RimPressureTable::items() const
{
    std::vector<RimPressureTableItem*> pressureTableItems = m_pressureTableItems.childObjects();

    // Sort by depth
    std::sort( pressureTableItems.begin(), pressureTableItems.end(), []( auto const& a, auto const& b ) {
        return a->depth() < b->depth();
    } );

    return pressureTableItems;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPressureTable::insertItem( RimPressureTableItem* insertBefore, RimPressureTableItem* item )
{
    size_t index = m_pressureTableItems.index( insertBefore );
    item->changed.connect( this, &RimPressureTable::onTableChanged );
    if ( index < m_pressureTableItems.size() )
        m_pressureTableItems.insert( index, item );
    else
        m_pressureTableItems.push_back( item );

    onTableChanged();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPressureTable::deleteItem( RimPressureTableItem* itemToDelete )
{
    m_pressureTableItems.removeChildObject( itemToDelete );
    delete itemToDelete;
    onTableChanged();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPressureTable::deleteAllItems()
{
    m_pressureTableItems.deleteAllChildObjects();
    onTableChanged();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPressureTable::defineCustomContextMenu( const caf::PdmFieldHandle* fieldNeedingMenu,
                                                QMenu*                     menu,
                                                QWidget*                   fieldEditorWidget )
{
    caf::CmdFeatureMenuBuilder menuBuilder;

    menuBuilder << "RicNewPressureTableItemFeature";
    menuBuilder << "Separator";
    menuBuilder << "RicDeletePressureTableItemFeature";

    menuBuilder.appendToMenu( menu );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPressureTable::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                              QString                    uiConfigName,
                                              caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_pressureTableItems )
    {
        auto tvAttribute = dynamic_cast<caf::PdmUiTableViewEditorAttribute*>( attribute );
        if ( tvAttribute )
        {
            tvAttribute->resizePolicy              = caf::PdmUiTableViewEditorAttribute::RESIZE_TO_FILL_CONTAINER;
            tvAttribute->alwaysEnforceResizePolicy = true;
            tvAttribute->minimumHeight             = 300;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPressureTable::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_pressureTableItems );
    uiOrdering.add( &m_pressureDate );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPressureTable::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPressureTable::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                         const QVariant&            oldValue,
                                         const QVariant&            newValue )
{
    if ( changedField == &m_pressureDate ) updatePressureDate();

    onTableChanged();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPressureTable::onTableChanged( const caf::SignalEmitter* emitter )
{
    changed.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPressureTable::initAfterRead()
{
    updatePressureDate();

    for ( auto item : items() )
    {
        item->changed.connect( this, &RimPressureTable::onTableChanged );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPressureTable::updatePressureDate()
{
    for ( auto item : items() )
        item->setPressureDate( m_pressureDate() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString& RimPressureTable::pressureDate() const
{
    return m_pressureDate();
}
