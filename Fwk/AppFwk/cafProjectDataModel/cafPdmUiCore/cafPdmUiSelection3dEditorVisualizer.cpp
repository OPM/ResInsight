//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron Solutions AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################

#include "cafPdmUiSelection3dEditorVisualizer.h"

//==================================================================================================
///
///
///
//==================================================================================================

#include "cafSelectionManager.h"

namespace caf
{
//--------------------------------------------------------------------------------------------------
/// The ownerViewer will take over ownership of this class. The ownerViewer is also the viewer distributed to
/// the 3dEditors created by this class to make them know where to exist.
//--------------------------------------------------------------------------------------------------
PdmUiSelection3dEditorVisualizer::PdmUiSelection3dEditorVisualizer( QWidget* ownerViewer )
    : m_ownerViewer( ownerViewer )
{
    this->setParent( ownerViewer ); // Makes this owned by the viewer.
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiSelection3dEditorVisualizer::~PdmUiSelection3dEditorVisualizer()
{
    for ( const auto& editor : m_active3DEditors )
    {
        delete editor;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiSelection3dEditorVisualizer::updateVisibleEditors()
{
    for ( const auto& editor : m_active3DEditors )
    {
        if ( editor ) editor->updateUi();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiSelection3dEditorVisualizer::onSelectionManagerSelectionChanged( const std::set<int>& changedSelectionLevels )
{
    if ( !changedSelectionLevels.count( 0 ) ) return;

    for ( const auto& editor : m_active3DEditors )
    {
        delete editor;
    }

    m_active3DEditors.clear();

    if ( !m_ownerViewer ) return;

    std::set<PdmUiItem*> totalSelection;
    for ( int selLevel : changedSelectionLevels )
    {
        std::vector<PdmUiItem*> items;
        caf::SelectionManager::instance()->selectedItems( items, selLevel );
        totalSelection.insert( items.begin(), items.end() );
    }

    for ( PdmUiItem* item : totalSelection )
    {
        QString editor3dTypeName = item->ui3dEditorTypeName( m_configName );
        if ( !editor3dTypeName.isEmpty() )
        {
            PdmObjectHandle* itemObject = dynamic_cast<PdmObjectHandle*>( item );
            if ( itemObject )
            {
                // Editor in main view
                {
                    PdmUi3dObjectEditorHandle* editor3d =
                        caf::Factory<PdmUi3dObjectEditorHandle, QString>::instance()->create( editor3dTypeName );
                    editor3d->setViewer( m_ownerViewer, false );
                    editor3d->setPdmObject( itemObject );
                    m_active3DEditors.emplace_back( editor3d );
                    editor3d->updateUi();
                }

                QVariant isComparisonActive = m_ownerViewer->property( "cafViewer_IsComparisonViewActive" );

                // Editor in comparison view
                if ( !isComparisonActive.isNull() && isComparisonActive.isValid() && isComparisonActive.toBool() )
                {
                    PdmUi3dObjectEditorHandle* editor3d =
                        caf::Factory<PdmUi3dObjectEditorHandle, QString>::instance()->create( editor3dTypeName );
                    editor3d->setViewer( m_ownerViewer, true );
                    editor3d->setPdmObject( itemObject );
                    m_active3DEditors.emplace_back( editor3d );
                    editor3d->updateUi();
                }
            }
        }
    }

    m_ownerViewer->update();
}

} // end namespace caf
