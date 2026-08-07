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

#pragma once

#include "RiaDefines.h"

#include <QString>

#include <set>

namespace caf
{
class PdmChildArrayFieldHandle;
class PdmObjectHandle;
} // namespace caf

//==================================================================================================
///
/// Helpers used to keep names unique among siblings in the project tree.
///
/// Siblings are the objects held by the same caf::PdmChildArrayField. Items and folders live in
/// two distinct child arrays (m_items and m_subCollections in caf::PdmNestedCollection), so a
/// folder and an item may carry the same name under the same parent. Names in different folders
/// never conflict.
///
/// All comparison is case sensitive.
///
//==================================================================================================
namespace RiaNameUniquenessTools
{
// Name of an object as shown in the project tree. Knows about the name field of surfaces,
// named objects and nested collections, and falls back to the user description field.
QString objectName( const caf::PdmObjectHandle* object );
void    setObjectName( caf::PdmObjectHandle* object, const QString& name );

// Names of the objects in a child array field, optionally excluding one object.
std::set<QString> namesInCollection( const caf::PdmChildArrayFieldHandle* childArrayField,
                                     const caf::PdmObjectHandle*          objectToExclude = nullptr );

// Names used by the other objects in the child array field holding this object.
std::set<QString> siblingNames( const caf::PdmObjectHandle* object );

// Appends _1, _2, ... to candidateName until the result is not present in takenNames.
QString makeUnique( const QString& candidateName, const std::set<QString>& takenNames );

bool    isUniqueAmongSiblings( const caf::PdmObjectHandle* object, const QString& candidateName );
QString makeUniqueAmongSiblings( const caf::PdmObjectHandle* object, const QString& candidateName );

// Renames the object if one of its siblings already carries the same name. The object must
// already be inserted into its parent collection. Returns the name in effect after the call.
QString ensureUniqueAmongSiblings( caf::PdmObjectHandle* object );

// Returns the name to apply when the user renames an object. If desiredName is already unique
// among the siblings of the object, it is returned unchanged. Otherwise an auto-generated unique
// name is returned, and the substitution is written to the log.
QString resolveRenameConflict( const caf::PdmObjectHandle* object, const QString& desiredName );

// For objects not yet inserted into a parent collection.
bool    isUniqueInCollection( const caf::PdmChildArrayFieldHandle* childArrayField, const QString& candidateName );
QString makeUniqueInCollection( const caf::PdmChildArrayFieldHandle* childArrayField, const QString& candidateName );

// The object in the child array field carrying this exact name, or nullptr.
caf::PdmObjectHandle* findInCollection( const caf::PdmChildArrayFieldHandle* childArrayField, const QString& name );

//==================================================================================================
/// Outcome of applying a name conflict policy. Exactly one of the three states applies:
///  - errorMessage set     : the policy is FAIL and the name is taken, abort the operation
///  - objectToReplace set  : the policy is OVERWRITE, the caller must delete this object
///  - neither set          : use nameToUse, which is unique
//==================================================================================================
struct ConflictResolution
{
    QString               nameToUse;
    caf::PdmObjectHandle* objectToReplace = nullptr;
    QString               errorMessage;
};

// Resolves a name conflict for a name about to be used in childArrayField. Deleting the object
// to replace is left to the caller, as the removal must go through the owning collection to keep
// views and referring objects in sync.
ConflictResolution applyConflictPolicy( const caf::PdmChildArrayFieldHandle* childArrayField,
                                        const QString&                       candidateName,
                                        RiaDefines::NameConflictPolicy       policy );

}; // namespace RiaNameUniquenessTools
