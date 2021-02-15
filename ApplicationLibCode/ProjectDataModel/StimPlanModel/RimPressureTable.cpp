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

    CAF_PDM_InitField( &m_useForInitialPressure, "UseForInitialPressure", false, "Use For Initial Pressure", "", "", "" );
    CAF_PDM_InitField( &m_useForPressure, "UseForPressure", false, "Use For Pressure", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_pressureTableItems, "Items", "Pressure Table Items", "", "", "" );
    m_pressureTableItems.uiCapability()->setUiEditorTypeName( caf::PdmUiTableViewEditor::uiEditorTypeName() );
    m_pressureTableItems.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );
    m_pressureTableItems.uiCapability()->setCustomContextMenuEnabled( true );
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
    std::vector<RimPressureTableItem*> attrs;

    for ( auto attr : m_pressureTableItems )
    {
        attrs.push_back( attr.p() );
    }
    return attrs;
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
    onTableChanged();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPressureTable::usePressureTableForProperty( RiaDefines::CurveProperty curveProperty ) const
{
    if ( curveProperty == RiaDefines::CurveProperty::INITIAL_PRESSURE )
        return m_useForInitialPressure();
    else if ( curveProperty == RiaDefines::CurveProperty::PRESSURE )
        return m_useForPressure();
    else
        return false;
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
    for ( auto item : items() )
    {
        item->changed.connect( this, &RimPressureTable::onTableChanged );
    }
}
