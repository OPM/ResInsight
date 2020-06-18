//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2019- Ceetron Solutions AS
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

#include "cafToggleItemsOnOthersOffFeature.h"

#include "cafToggleItemsFeatureImpl.h"

#include "cafSelectionManager.h"

#include "cafPdmObject.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmUiItem.h"
#include <QAction>

namespace caf
{
CAF_CMD_SOURCE_INIT( ToggleItemsOnOthersOffFeature, "cafToggleItemsOnOthersOffFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool ToggleItemsOnOthersOffFeature::isCommandEnabled()
{
    std::vector<caf::PdmObject*> selectedObjects;
    caf::SelectionManager::instance()->objectsByType( &selectedObjects );

    caf::PdmFieldHandle*               commonParent = verifySameParentForSelection( selectedObjects );
    std::vector<caf::PdmObjectHandle*> children     = childObjects( commonParent );

    return commonParent != nullptr && children.size() > 0 && objectToggleField( children.front() ) &&
           children.size() > selectedObjects.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ToggleItemsOnOthersOffFeature::onActionTriggered( bool isChecked )
{
    std::vector<caf::PdmObject*> selectedObjects;
    caf::SelectionManager::instance()->objectsByType( &selectedObjects );

    // First toggle off all siblings

    caf::PdmFieldHandle* commonParent = verifySameParentForSelection( selectedObjects );

    for ( caf::PdmObjectHandle* child : childObjects( commonParent ) )
    {
        caf::PdmField<bool>* field = objectToggleField( child );

        if ( field )
        {
            field->setValueWithFieldChanged( false );
        }
    }

    // Then toggle on the selected item(s)
    for ( caf::PdmObject* selectedObject : selectedObjects )
    {
        caf::PdmField<bool>* field = dynamic_cast<caf::PdmField<bool>*>( selectedObject->objectToggleField() );

        field->setValueWithFieldChanged( true );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ToggleItemsOnOthersOffFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "On - Others Off" );

    actionToSetup->setIcon( QIcon( ":/cafCommandFeatures/ToggleOnOthersOffL16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle*
    ToggleItemsOnOthersOffFeature::verifySameParentForSelection( const std::vector<caf::PdmObject*>& selection )
{
    caf::PdmFieldHandle* sameParent = nullptr;

    for ( caf::PdmObject* obj : selection )
    {
        caf::PdmFieldHandle* parent = obj->parentField();
        if ( parent )
        {
            if ( !sameParent )
            {
                sameParent = parent;
            }
            else if ( parent != sameParent )
            {
                // Different parents detected

                return nullptr;
            }
        }
    }
    return sameParent;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmObjectHandle*> ToggleItemsOnOthersOffFeature::childObjects( caf::PdmFieldHandle* parent )
{
    std::vector<caf::PdmObjectHandle*> children;
    if ( parent )
    {
        parent->childObjects( &children );
    }
    return children;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmField<bool>* ToggleItemsOnOthersOffFeature::objectToggleField( caf::PdmObjectHandle* objectHandle )
{
    caf::PdmUiObjectHandle* childUiObject = uiObj( objectHandle );
    if ( childUiObject && childUiObject->objectToggleField() )
    {
        return dynamic_cast<caf::PdmField<bool>*>( childUiObject->objectToggleField() );
    }
    return nullptr;
}

} // namespace caf