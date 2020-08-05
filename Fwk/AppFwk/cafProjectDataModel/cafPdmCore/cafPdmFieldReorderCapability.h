//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2020- Ceetron Solutions AS
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
#pragma once

#include "cafPdmFieldCapability.h"
#include "cafPdmPtrArrayFieldHandle.h"
#include "cafSignal.h"

namespace caf
{
class PdmFieldReorderCapability : public PdmFieldCapability, public SignalEmitter, public SignalObserver
{
public:
    Signal<> orderChanged;

public:
    PdmFieldReorderCapability( PdmPtrArrayFieldHandle* field, bool giveOwnership );

    bool itemCanBeMovedUp( size_t index ) const;
    bool itemCanBeMovedDown( size_t index ) const;

    bool moveItemUp( size_t index );
    bool moveItemDown( size_t index );

    static PdmFieldReorderCapability* addToField( PdmPtrArrayFieldHandle* field );
    static bool                       fieldIsReorderable( PdmPtrArrayFieldHandle* field );

    void onMoveItemUp( const SignalEmitter* emitter, size_t index );
    void onMoveItemDown( const SignalEmitter* emitter, size_t index );

private:
    PdmPtrArrayFieldHandle* m_field;
};

}; // namespace caf
