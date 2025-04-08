/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RicToggleItemsOnOthersOffFeature.h"

#include "RicToggleItemsFeatureImpl.h"

#include "cafSelectionManager.h"

#include "cafPdmObject.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmUiItem.h"
#include <QAction>

CAF_CMD_SOURCE_INIT( RicToggleItemsOnOthersOffFeature, "RicToggleItemsOnOthersOffFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicToggleItemsOnOthersOffFeature::isCommandEnabled() const
{
    const auto selectedObjects = caf::SelectionManager::instance()->objectsByType<caf::PdmObject>();

    caf::PdmFieldHandle*               commonParent = commonParentForAllSelections( selectedObjects );
    std::vector<caf::PdmObjectHandle*> children     = childObjects( commonParent );

    return commonParent != nullptr && !children.empty() && objectToggleField( children.front() ) && children.size() > selectedObjects.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicToggleItemsOnOthersOffFeature::onActionTriggered( bool isChecked )
{
    const auto selectedObjects = caf::SelectionManager::instance()->objectsByType<caf::PdmObject>();

    // First toggle off all siblings
    caf::PdmFieldHandle* commonParent = commonParentForAllSelections( selectedObjects );

    caf::PdmFieldHandle* firstField = nullptr;

    for ( caf::PdmObjectHandle* child : childObjects( commonParent ) )
    {
        caf::PdmField<bool>* field = objectToggleField( child );

        if ( field )
        {
            if ( !firstField ) firstField = field;

            // Avoid calling setValueWithFieldChanged() here, as this potentially can trigger heavy computations. Assume
            // that the update logic is sufficient when setting the selected objects.
            field->setValue( false );
        }
    }

    // Then toggle on the selected item(s)
    for ( caf::PdmObject* selectedObject : selectedObjects )
    {
        caf::PdmField<bool>* field = dynamic_cast<caf::PdmField<bool>*>( selectedObject->objectToggleField() );

        field->setValueWithFieldChanged( true );
    }

    // If multiple fields are updated, we call onChildrenUpdated() on the owner of the first field
    // Example: Trigger replot of curves when multiple curves are toggled
    auto [ownerOfChildArrayField, childArrayFieldHandle] = RicToggleItemsFeatureImpl::findOwnerAndChildArrayField( firstField );
    if ( ownerOfChildArrayField && childArrayFieldHandle )
    {
        std::vector<caf::PdmObjectHandle*> objs;
        ownerOfChildArrayField->onChildrenUpdated( childArrayFieldHandle, objs );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicToggleItemsOnOthersOffFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "On - Others Off" );

    actionToSetup->setIcon( QIcon( ":/ToggleOnOthersOff16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RicToggleItemsOnOthersOffFeature::commonParentForAllSelections( const std::vector<caf::PdmObject*>& selectedObjects )
{
    caf::PdmFieldHandle* commonParent = nullptr;

    for ( caf::PdmObject* obj : selectedObjects )
    {
        caf::PdmFieldHandle* parent = obj->parentField();
        if ( parent )
        {
            if ( !commonParent )
            {
                commonParent = parent;
            }
            else if ( parent != commonParent )
            {
                // Different parents detected
                return nullptr;
            }
        }
    }
    return commonParent;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmObjectHandle*> RicToggleItemsOnOthersOffFeature::childObjects( caf::PdmFieldHandle* parent )
{
    if ( parent )
    {
        return parent->children();
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmField<bool>* RicToggleItemsOnOthersOffFeature::objectToggleField( caf::PdmObjectHandle* objectHandle )
{
    caf::PdmUiObjectHandle* childUiObject = uiObj( objectHandle );
    if ( childUiObject && childUiObject->objectToggleField() )
    {
        return dynamic_cast<caf::PdmField<bool>*>( childUiObject->objectToggleField() );
    }
    return nullptr;
}
