/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "RiaNameUniquenessTools.h"

#include "RiaLogging.h"

#include "RimNamedObject.h"
#include "RimSurface.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmNestedCollectionBase.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmUiObjectHandle.h"
#include "cafPdmValueField.h"

#include <QVariant>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaNameUniquenessTools::objectName( const caf::PdmObjectHandle* object )
{
    if ( !object ) return {};

    auto* mutableObject = const_cast<caf::PdmObjectHandle*>( object );

    if ( auto* surface = dynamic_cast<RimSurface*>( mutableObject ) ) return surface->userDescription();
    if ( auto* namedObject = dynamic_cast<RimNamedObject*>( mutableObject ) ) return namedObject->name();
    if ( auto* collection = dynamic_cast<caf::PdmNestedCollectionBase*>( mutableObject ) ) return collection->collectionName();

    if ( auto* uiObjectHandle = caf::uiObj( object ) )
    {
        if ( auto* valueField = dynamic_cast<caf::PdmValueField*>( uiObjectHandle->userDescriptionField() ) )
        {
            return valueField->toQVariant().toString();
        }
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaNameUniquenessTools::setObjectName( caf::PdmObjectHandle* object, const QString& name )
{
    if ( !object ) return;

    if ( auto* surface = dynamic_cast<RimSurface*>( object ) )
    {
        surface->setUserDescription( name );
        return;
    }

    if ( auto* namedObject = dynamic_cast<RimNamedObject*>( object ) )
    {
        namedObject->setName( name );
        return;
    }

    if ( auto* collection = dynamic_cast<caf::PdmNestedCollectionBase*>( object ) )
    {
        collection->setCollectionName( name );
        return;
    }

    if ( auto* uiObjectHandle = caf::uiObj( object ) )
    {
        if ( auto* valueField = dynamic_cast<caf::PdmValueField*>( uiObjectHandle->userDescriptionField() ) )
        {
            valueField->setFromQVariant( name );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QString> RiaNameUniquenessTools::namesInCollection( const caf::PdmChildArrayFieldHandle* childArrayField,
                                                             const caf::PdmObjectHandle*          objectToExclude )
{
    std::set<QString> names;
    if ( !childArrayField ) return names;

    auto* mutableField = const_cast<caf::PdmChildArrayFieldHandle*>( childArrayField );
    for ( size_t i = 0; i < mutableField->size(); i++ )
    {
        auto* child = mutableField->at( i );
        if ( !child || child == objectToExclude ) continue;

        names.insert( objectName( child ) );
    }

    return names;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QString> RiaNameUniquenessTools::siblingNames( const caf::PdmObjectHandle* object )
{
    if ( !object ) return {};

    auto* childArrayField = dynamic_cast<caf::PdmChildArrayFieldHandle*>( object->parentField() );
    if ( !childArrayField ) return {};

    return namesInCollection( childArrayField, object );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaNameUniquenessTools::makeUnique( const QString& candidateName, const std::set<QString>& takenNames )
{
    // An empty name means the object has no name of its own, and derives its tree label from
    // other properties (a grid case surface is labelled from its case and K index). Numbering
    // those would only produce labels like "_1 - K:3".
    if ( candidateName.isEmpty() ) return candidateName;

    if ( !takenNames.contains( candidateName ) ) return candidateName;

    int     counter = 1;
    QString uniqueName;
    do
    {
        uniqueName = QString( "%1_%2" ).arg( candidateName ).arg( counter++ );
    } while ( takenNames.contains( uniqueName ) );

    return uniqueName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaNameUniquenessTools::isUniqueAmongSiblings( const caf::PdmObjectHandle* object, const QString& candidateName )
{
    if ( candidateName.isEmpty() ) return true;

    return !siblingNames( object ).contains( candidateName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaNameUniquenessTools::makeUniqueAmongSiblings( const caf::PdmObjectHandle* object, const QString& candidateName )
{
    return makeUnique( candidateName, siblingNames( object ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaNameUniquenessTools::ensureUniqueAmongSiblings( caf::PdmObjectHandle* object )
{
    const QString currentName = objectName( object );
    const QString uniqueName  = makeUniqueAmongSiblings( object, currentName );

    if ( uniqueName != currentName ) setObjectName( object, uniqueName );

    return uniqueName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaNameUniquenessTools::resolveRenameConflict( const caf::PdmObjectHandle* object, const QString& desiredName )
{
    const QString uniqueName = makeUniqueAmongSiblings( object, desiredName );

    if ( uniqueName != desiredName )
    {
        RiaLogging::info(
            QString( "\"%1\" already exists in this folder, using \"%2\" instead." ).arg( desiredName ).arg( uniqueName ).toStdString() );
    }

    return uniqueName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaNameUniquenessTools::isUniqueInCollection( const caf::PdmChildArrayFieldHandle* childArrayField, const QString& candidateName )
{
    if ( candidateName.isEmpty() ) return true;

    return !namesInCollection( childArrayField ).contains( candidateName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaNameUniquenessTools::makeUniqueInCollection( const caf::PdmChildArrayFieldHandle* childArrayField, const QString& candidateName )
{
    return makeUnique( candidateName, namesInCollection( childArrayField ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* RiaNameUniquenessTools::findInCollection( const caf::PdmChildArrayFieldHandle* childArrayField, const QString& name )
{
    if ( !childArrayField ) return nullptr;

    auto* mutableField = const_cast<caf::PdmChildArrayFieldHandle*>( childArrayField );
    for ( size_t i = 0; i < mutableField->size(); i++ )
    {
        auto* child = mutableField->at( i );
        if ( child && objectName( child ) == name ) return child;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaNameUniquenessTools::ConflictResolution RiaNameUniquenessTools::applyConflictPolicy( const caf::PdmChildArrayFieldHandle* childArrayField,
                                                                                        const QString&                 candidateName,
                                                                                        RiaDefines::NameConflictPolicy policy )
{
    ConflictResolution resolution;
    resolution.nameToUse = candidateName;

    if ( candidateName.isEmpty() ) return resolution;

    auto* existingObject = findInCollection( childArrayField, candidateName );
    if ( !existingObject ) return resolution;

    switch ( policy )
    {
        case RiaDefines::NameConflictPolicy::AUTO_RENAME:
            resolution.nameToUse = makeUniqueInCollection( childArrayField, candidateName );
            break;

        case RiaDefines::NameConflictPolicy::OVERWRITE:
            resolution.objectToReplace = existingObject;
            break;

        case RiaDefines::NameConflictPolicy::FAIL:
        default:
            resolution.errorMessage = QString( "An item named '%1' already exists in this folder" ).arg( candidateName );
            break;
    }

    return resolution;
}
