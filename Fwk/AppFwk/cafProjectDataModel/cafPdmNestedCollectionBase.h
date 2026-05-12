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

//==================================================================================================
///
/// Non-templated PdmObject base class for tree-shaped containers.
///
/// Provides the type-erased interface (collection name, add-new-subcollection, leaf check) that
/// generic command features and script methods need. The templated caf::PdmNestedCollection<>
/// derives from this and supplies typed storage and accessors.
///
/// Concrete (instantiable) so the CAF factory can create a dummy instance for the Python
/// generator. The base's addNewSubCollection() is a no-op returning nullptr; the templated
/// derived class overrides it with a real implementation.
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

    // Whether this container accepts sub-collections. Leaf containers (e.g. file-backed
    // folders) override to return false; generic UI / script features hide the corresponding
    // action when this is false.
    virtual bool canAddSubCollection() const { return true; }

    // Creates a subcollection, adds it to this collection, and returns it. Default returns
    // nullptr; concrete derivations (typically PdmNestedCollection<>) override.
    virtual PdmObject* addNewSubCollection() { return nullptr; }

    // Marks this instance as the topmost folder: hides and disables IO on the collection name,
    // makes the object non-deletable.
    void setAsTopmostFolder();

protected:
    PdmFieldHandle* userDescriptionField() override;

    PdmField<QString> m_collectionName;

    bool m_isTopmostFolder;
};

} // namespace caf
