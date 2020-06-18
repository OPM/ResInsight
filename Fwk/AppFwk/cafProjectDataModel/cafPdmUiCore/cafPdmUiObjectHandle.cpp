#include "cafPdmUiObjectHandle.h"

#include "cafAssert.h"
#include "cafPdmFieldHandle.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmUiFieldHandle.h"
#include "cafPdmUiOrdering.h"
#include "cafPdmUiTreeOrdering.h"

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiObjectHandle::PdmUiObjectHandle( PdmObjectHandle* owner, bool giveOwnership )
{
    m_owner = owner;
    m_owner->addCapability( this, giveOwnership );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiObjectHandle* uiObj( const PdmObjectHandle* obj )
{
    if ( !obj ) return nullptr;
    PdmUiObjectHandle* uiObject = obj->capability<PdmUiObjectHandle>();
    CAF_ASSERT( uiObject );
    return uiObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiObjectHandle::uiOrdering( const QString& uiConfigName, PdmUiOrdering& uiOrdering )
{
    // Restore state for includeRemainingFields, as this flag
    // can be changed in defineUiOrdering()
    bool includeRemaining_originalState = uiOrdering.isIncludingRemainingFields();

    this->defineUiOrdering( uiConfigName, uiOrdering );
    if ( uiOrdering.isIncludingRemainingFields() )
    {
        // Add the remaining Fields To UiConfig
        std::vector<PdmFieldHandle*> fields;
        m_owner->fields( fields );
        for ( size_t i = 0; i < fields.size(); ++i )
        {
            PdmUiFieldHandle* field = fields[i]->uiCapability();
            if ( !uiOrdering.contains( field ) )
            {
                uiOrdering.add( field->fieldHandle() );
            }
        }
    }

    // Restore incoming value
    uiOrdering.skipRemainingFields( !includeRemaining_originalState );

    CAF_ASSERT( includeRemaining_originalState == uiOrdering.isIncludingRemainingFields() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiObjectHandle::editorAttribute( const PdmFieldHandle* field,
                                         const QString&        uiConfigName,
                                         PdmUiEditorAttribute* attribute )
{
    this->defineEditorAttribute( field, uiConfigName, attribute );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiObjectHandle::objectEditorAttribute( const QString& uiConfigName, PdmUiEditorAttribute* attribute )
{
    this->defineObjectEditorAttribute( uiConfigName, attribute );
}

//--------------------------------------------------------------------------------------------------
/// This method creates a tree-representation of the object hierarchy starting at this
/// object to be used for a tree view.
/// This method calls the optional virtual user defined method "defineUiTreeOrdering" to customize the
/// order and content of the children directly below each object. If this method does nothing,
/// the default behavior applies: Add all fields that contains objects, and their objects.
///
/// The caller is responsible to delete the returned PdmUiTreeOrdering
//--------------------------------------------------------------------------------------------------
PdmUiTreeOrdering* PdmUiObjectHandle::uiTreeOrdering( const QString& uiConfigName /*= ""*/ ) const
{
    CAF_ASSERT( this ); // This method actually is possible to call on a NULL ptr without getting a crash, so we assert
                        // instead.

    PdmUiTreeOrdering* uiTreeOrdering = new PdmUiTreeOrdering( nullptr, m_owner );

    expandUiTree( uiTreeOrdering, uiConfigName );

    return uiTreeOrdering;
}

//--------------------------------------------------------------------------------------------------
/// Adds the direct children of this PdmObject to the PdmUiTree according to
/// the default rules. Add all fields that contains objects, and their objects.
/// Takes into account the control variables regarding this:
/// PdmField::isUiHidden
/// PdmField::isUiChildrenHidden
/// And whether the fields and objects are already added by the user.
//--------------------------------------------------------------------------------------------------
void PdmUiObjectHandle::addDefaultUiTreeChildren( PdmUiTreeOrdering* uiTreeOrdering )
{
#if 1
    if ( uiTreeOrdering->isIncludingRemainingChildren() )
    {
        // Add the remaining Fields To UiConfig
        std::vector<PdmFieldHandle*> fields;
        m_owner->fields( fields );

        for ( size_t fIdx = 0; fIdx < fields.size(); ++fIdx )
        {
            if ( fields[fIdx]->hasChildObjects() && !uiTreeOrdering->containsField( fields[fIdx] ) )
            {
                if ( fields[fIdx]->uiCapability()->isUiHidden() && !fields[fIdx]->uiCapability()->isUiTreeChildrenHidden() )
                {
                    std::vector<PdmObjectHandle*> children;
                    fields[fIdx]->childObjects( &children );

                    std::set<PdmObjectHandle*> objectsAddedByApplication;
                    for ( int i = 0; i < uiTreeOrdering->childCount(); i++ )
                    {
                        if ( uiTreeOrdering->child( i )->isRepresentingObject() )
                        {
                            objectsAddedByApplication.insert( uiTreeOrdering->child( i )->object() );
                        }
                    }

                    for ( size_t cIdx = 0; cIdx < children.size(); cIdx++ )
                    {
                        if ( children[cIdx] )
                        {
                            bool                                 isAlreadyAdded = false;
                            std::set<PdmObjectHandle*>::iterator it = objectsAddedByApplication.find( children[cIdx] );
                            if ( it != objectsAddedByApplication.end() )
                            {
                                isAlreadyAdded = true;
                                break;
                            }

                            if ( !isAlreadyAdded )
                            {
                                uiTreeOrdering->add( children[cIdx] );
                            }
                        }
                    }
                }
                else if ( !fields[fIdx]->uiCapability()->isUiHidden() )
                {
                    uiTreeOrdering->add( fields[fIdx] );
                }
            }
        }
    }
#endif
}

//--------------------------------------------------------------------------------------------------
/// Builds the sPdmUiTree for all the children of @param root recursively, and stores the result
/// in root
//--------------------------------------------------------------------------------------------------
void PdmUiObjectHandle::expandUiTree( PdmUiTreeOrdering* root, const QString& uiConfigName /*= "" */ )
{
#if 1
    if ( !root || !root->isValid() ) return;

    if ( root->childCount() > 0 )
    {
        for ( int cIdx = 0; cIdx < root->childCount(); ++cIdx )
        {
            PdmUiTreeOrdering* child = root->child( cIdx );
            if ( child->isValid() && !child->ignoreSubTree() )
            {
                expandUiTree( child, uiConfigName );
            }
        }
    }
    else //( root->childCount() == 0) // This means that no one has tried to expand it.
    {
        if ( !root->ignoreSubTree() )
        {
            if ( root->isRepresentingField() && !root->field()->uiCapability()->isUiTreeChildrenHidden( uiConfigName ) )
            {
                std::vector<PdmObjectHandle*> fieldsChildObjects;
                root->field()->childObjects( &fieldsChildObjects );
                for ( size_t cIdx = 0; cIdx < fieldsChildObjects.size(); ++cIdx )
                {
                    PdmObjectHandle* childObject = fieldsChildObjects[cIdx];
                    if ( childObject )
                    {
                        root->appendChild( uiObj( childObject )->uiTreeOrdering( uiConfigName ) );
                    }
                }
            }
            else if ( root->isRepresentingObject() &&
                      !root->object()->uiCapability()->isUiTreeChildrenHidden( uiConfigName ) )
            {
                uiObj( root->object() )->defineUiTreeOrdering( *root, uiConfigName );
                uiObj( root->object() )->addDefaultUiTreeChildren( root );
                if ( root->childCount() )
                {
                    expandUiTree( root, uiConfigName );
                }
            }
        }
    }
#endif
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiObjectHandle::updateUiIconFromToggleField()
{
    if ( objectToggleField() )
    {
        PdmUiFieldHandle* uiFieldHandle = objectToggleField()->uiCapability();
        if ( uiFieldHandle )
        {
            bool active = uiFieldHandle->uiValue().toBool();
            updateUiIconFromState( active );
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Implementation of uiCapability() defined in cafPdmObjectHandle.h
//--------------------------------------------------------------------------------------------------
PdmUiObjectHandle* PdmObjectHandle::uiCapability() const
{
    PdmUiObjectHandle* uiField = capability<PdmUiObjectHandle>();
    CAF_ASSERT( uiField );

    return uiField;
}

} // End namespace caf
