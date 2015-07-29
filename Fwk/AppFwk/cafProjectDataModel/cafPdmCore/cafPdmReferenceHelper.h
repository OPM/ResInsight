//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
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

#include "cafPdmObjectHandle.h"
#include "cafPdmPointer.h"

class QStringList;

namespace caf 
{

//==================================================================================================
/// 
//==================================================================================================
class PdmReferenceHelper
{
public:
    static PdmObjectHandle* findRoot(PdmObjectHandle* obj);
    static PdmObjectHandle* findRoot(PdmFieldHandle* field);

    static QString          referenceFromRootToField(PdmObjectHandle* root, PdmFieldHandle* field);
    static QString          referenceFromRootToObject(PdmObjectHandle* root, PdmObjectHandle* obj);
    static PdmObjectHandle* objectFromReference(PdmObjectHandle* root, const QString& reference);
    static PdmFieldHandle*  fieldFromReference(PdmObjectHandle* root, const QString& reference);

    static QString          referenceFromFieldToObject(PdmFieldHandle* fromField, PdmObjectHandle* toObj);
    static PdmObjectHandle* objectFromFieldReference(PdmFieldHandle* fromField, const QString& reference);

private:
    static QStringList      referenceFromRootToObjectAsStringList(PdmObjectHandle* root, PdmObjectHandle* obj);
    static PdmObjectHandle* objectFromReferenceStringList(PdmObjectHandle* root, const QStringList& reference);

    static PdmFieldHandle*  findField(PdmObjectHandle* object, const QString& fieldKeyword);

};



} // end namespace caf
