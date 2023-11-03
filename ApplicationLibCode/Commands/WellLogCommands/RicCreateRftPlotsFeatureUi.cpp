/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023 Equinor ASA
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

#include "RicCreateRftPlotsFeatureUi.h"

#include "cafPdmUiTreeSelectionEditor.h"

CAF_PDM_SOURCE_INIT( RicCreateRftPlotsFeatureUi, "RicCreateRftPlotsFeatureUi" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicCreateRftPlotsFeatureUi::RicCreateRftPlotsFeatureUi()
{
    CAF_PDM_InitObject( "RicCreateRftPlotsFeatureUi" );

    CAF_PDM_InitFieldNoDefault( &m_selectedWellNames, "SelectedWellNames", "Well Names" );
    m_selectedWellNames.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateRftPlotsFeatureUi::setAllWellNames( const std::vector<QString>& wellNames )
{
    m_allWellNames = wellNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RicCreateRftPlotsFeatureUi::selectedWellNames() const
{
    return m_selectedWellNames();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RicCreateRftPlotsFeatureUi::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;
    if ( fieldNeedingOptions == &m_selectedWellNames )
    {
        for ( const auto& wellName : m_allWellNames )
        {
            options.append( caf::PdmOptionItemInfo( wellName, wellName ) );
        }
    }

    return options;
}
