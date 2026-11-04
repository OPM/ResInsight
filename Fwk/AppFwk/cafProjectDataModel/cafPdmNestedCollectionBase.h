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

#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <QString>

namespace caf
{
class PdmChildArrayFieldHandle;

//==================================================================================================
///
/// Non-templated PdmObject base class for tree-shaped containers.
///
/// Provides the type-erased interface (collection name, add-new-subcollection, leaf check) that
/// generic command features and script methods need. The templated caf::PdmNestedCollection<>
/// derives from this and supplies typed storage and accessors.
///
/// Abstract: the sub-collection type, its creation and the field holding the sub-collections are
/// pure virtuals that caf::PdmNestedCollection<> implements. Registered with
/// CAF_PDM_ABSTRACT_SOURCE_INIT so the Python generator still places the generic methods here.
///
//==================================================================================================
class PdmNestedCollectionBase : public PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    PdmNestedCollectionBase();
    ~PdmNestedCollectionBase() override;

    QString collectionName() const;
    void    setCollectionName( const QString& name );

    // Name of the folder as the user sees it. Defaults to collectionName(); a folder whose name
    // falls back to something else when the stored name is empty (e.g. a file name) overrides this.
    virtual QString displayName() const;

    // Whether this container accepts sub-collections. Leaf containers (e.g. file-backed
    // folders) override to return false; generic UI / script features hide the corresponding
    // action when this is false.
    virtual bool canAddSubCollection() const { return true; }

    // Class keyword of the folder created by addNewSubCollection(). Empty when canAddSubCollection()
    // is false. Has no side effects, so the Python generator can read the folder type from it.
    virtual QString subCollectionClassKeyword() const = 0;

    // Creates a subcollection, adds it to this collection, and returns it. Returns nullptr when
    // canAddSubCollection() is false, otherwise delegates to createSubCollection().
    PdmNestedCollectionBase* addNewSubCollection();

    // Type-erased access to the field holding the sub-collections, for generic code that must
    // inspect the folders without knowing the concrete collection type.
    virtual PdmChildArrayFieldHandle* subCollectionsField() = 0;

    // Marks this instance as the topmost folder: hides and disables IO on the collection name,
    // makes the object non-deletable.
    void setAsTopmostFolder();

protected:
    // Creates the subcollection, adds it to this collection, and returns it. Called by
    // addNewSubCollection() when canAddSubCollection() is true.
    virtual PdmNestedCollectionBase* createSubCollection() = 0;

    PdmFieldHandle* userDescriptionField() override;

    PdmField<QString> m_collectionName;

    bool m_isTopmostFolder;
};

} // namespace caf
