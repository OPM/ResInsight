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

#include "RimCheckableNamedObject.h"

#include "cafPdmPtrField.h"

class RimWellPath;

//==================================================================================================
///
/// Per-view visibility wrapper around a global RimWellPath, following the same in-view
/// mirroring pattern as RimPolygonInView/RimSurfaceInView.
///
/// Reuses the "Name" field inherited from RimNamedObject (via RimCheckableNamedObject) rather
/// than declaring a separate name field, since a derived class field cannot reuse the same PDM
/// keyword as an inherited field.
//==================================================================================================
class RimWellPathInView : public RimCheckableNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    using SourceItemT = RimWellPath;

    RimWellPathInView();

    RimWellPath* wellPath() const;
    RimWellPath* sourceItem() const;
    void         setWellPath( RimWellPath* wellPath );

protected:
    void initAfterRead() override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

private:
    caf::PdmPtrField<RimWellPath*> m_wellPath;
};
