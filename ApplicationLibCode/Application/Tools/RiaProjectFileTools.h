/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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
#include "cafPdmObjectHandle.h"
#include "cafPdmXmlFieldHandle.h"

#include <QString>

#include <vector>

class RiaVariableMapper;
class RimProject;

//==================================================================================================
//
//==================================================================================================
namespace RiaProjectFileTools
{
bool isCandidateVersionNewerThanOther( const QString& candidateProjectFileVersion, const QString& otherProjectFileVersion );

template <typename T>
std::vector<T*> writableFieldContent( const caf::PdmObjectHandle* object )
{
    if ( !object ) return {};

    std::vector<T*> fieldContents;

    std::vector<caf::PdmObjectHandle*> children;
    for ( const auto& field : object->fields() )
    {
        if ( !field ) continue;

        if ( field->xmlCapability() && !field->xmlCapability()->isIOWritable() ) continue;

        caf::PdmField<T>* typedField = dynamic_cast<caf::PdmField<T>*>( field );
        if ( typedField ) fieldContents.push_back( &typedField->v() );

        caf::PdmField<std::vector<T>>* typedFieldInVector = dynamic_cast<caf::PdmField<std::vector<T>>*>( field );
        if ( typedFieldInVector )
        {
            for ( T& typedFieldFromVector : typedFieldInVector->v() )
            {
                fieldContents.push_back( &typedFieldFromVector );
            }
        }

        auto other = field->children();
        children.insert( children.end(), other.begin(), other.end() );
    }

    for ( const auto& child : children )
    {
        auto childObjects = writableFieldContent<T>( child );
        fieldContents.insert( fieldContents.end(), childObjects.begin(), childObjects.end() );
    }

    return fieldContents;
}

QString transferPathsToGlobalPathList( RimProject* project );
void    distributePathsFromGlobalPathList( RimProject* project, const QString& pathList );
QString updatedFilePathFromPathId( QString filePath, RiaVariableMapper* pathListMapper );

// Public to be able to unit test function
void decodeVersionString( const QString& projectFileVersion, int* majorVersion, int* minorVersion, int* patch, int* developmentId );
}; // namespace RiaProjectFileTools
