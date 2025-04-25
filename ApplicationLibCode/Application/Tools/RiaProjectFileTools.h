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

//==================================================================================================
//
//==================================================================================================
class RiaProjectFileTools
{
public:
    static bool isCandidateVersionNewerThanOther( const QString& candidateProjectFileVersion, const QString& otherProjectFileVersion );

    // Public to be able to unit test function, not intended to be used
    static void decodeVersionString( const QString& projectFileVersion, int* majorVersion, int* minorVersion, int* patch, int* developmentId );

    template <typename T>
    static void fieldContentsByType( const caf::PdmObjectHandle* object, std::vector<T*>& fieldContents )
    {
        if ( !object ) return;

        std::vector<caf::PdmFieldHandle*> allFieldsInObject = object->fields();

        std::vector<caf::PdmObjectHandle*> children;

        for ( const auto& field : allFieldsInObject )
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
            fieldContentsByType( child, fieldContents );
        }
    }

private:
    static bool isCandidateNewerThanOther( int candidateMajorVersion,
                                           int candidateMinorVersion,
                                           int candidatePatchNumber,
                                           int candidateDevelopmentId,
                                           int otherMajorVersion,
                                           int otherMinorVersion,
                                           int otherPatchNumber,
                                           int otherDevelopmentId );

    static QString stringOfDigits( const QString& string );
};
