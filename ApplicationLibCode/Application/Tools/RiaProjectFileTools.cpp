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

#include "RiaProjectFileTools.h"

#include <QStringList>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaProjectFileTools::isCandidateVersionNewerThanOther( const QString& candidateProjectFileVersion, const QString& projectFileVersion )
{
    int candidateMajorVersion  = 0;
    int candidateMinorVersion  = 0;
    int candidatePatchNumber   = -1;
    int candidateDevelopmentId = -1;

    RiaProjectFileTools::decodeVersionString( candidateProjectFileVersion,
                                              &candidateMajorVersion,
                                              &candidateMinorVersion,
                                              &candidatePatchNumber,
                                              &candidateDevelopmentId );

    int majorVersion  = 0;
    int minorVersion  = 0;
    int patchNumber   = -1;
    int developmentId = -1;

    RiaProjectFileTools::decodeVersionString( projectFileVersion, &majorVersion, &minorVersion, &patchNumber, &developmentId );

    return RiaProjectFileTools::isCandidateNewerThanOther( candidateMajorVersion,
                                                           candidateMinorVersion,
                                                           candidatePatchNumber,
                                                           candidateDevelopmentId,
                                                           majorVersion,
                                                           minorVersion,
                                                           patchNumber,
                                                           developmentId );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaProjectFileTools::decodeVersionString( const QString& projectFileVersion, int* majorVersion, int* minorVersion, int* patch, int* developmentId )
{
    if ( projectFileVersion.isEmpty() ) return;

    QStringList subStrings = projectFileVersion.split( "." );

    if ( !subStrings.empty() )
    {
        *majorVersion = subStrings[0].toInt();
    }

    if ( subStrings.size() > 1 )
    {
        *minorVersion = subStrings[1].toInt();
    }

    if ( subStrings.size() > 2 )
    {
        QString candidate           = subStrings[2];
        QString candidateDigitsOnly = RiaProjectFileTools::stringOfDigits( candidate );

        *patch = candidateDigitsOnly.toInt();
    }

    if ( subStrings.size() > 3 )
    {
        QString candidate           = subStrings.back();
        QString candidateDigitsOnly = RiaProjectFileTools::stringOfDigits( candidate );

        *developmentId = candidateDigitsOnly.toInt();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaProjectFileTools::isCandidateNewerThanOther( int candidateMajorVersion,
                                                     int candidateMinorVersion,
                                                     int candidatePatchNumber,
                                                     int candidateDevelopmentId,
                                                     int otherMajorVersion,
                                                     int otherMinorVersion,
                                                     int otherPatchNumber,
                                                     int otherDevelopmentId )
{
    if ( candidateMajorVersion != otherMajorVersion )
    {
        return ( candidateMajorVersion > otherMajorVersion );
    }

    if ( candidateMinorVersion != otherMinorVersion )
    {
        return ( candidateMinorVersion > otherMinorVersion );
    }

    // Early exit if a patch number is undefined
    if ( candidatePatchNumber == -1 || otherPatchNumber == -1 ) return false;

    if ( candidatePatchNumber != otherPatchNumber )
    {
        return ( candidatePatchNumber > otherPatchNumber );
    }

    // Early exit if a development number is undefined
    if ( candidateDevelopmentId == -1 && otherDevelopmentId == -1 ) return false;

    if ( candidateDevelopmentId != otherDevelopmentId )
    {
        return ( candidateDevelopmentId > otherDevelopmentId );
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaProjectFileTools::stringOfDigits( const QString& string )
{
    QString digitsOnly;

    for ( const auto& c : string )
    {
        if ( c.isDigit() )
        {
            digitsOnly += c;
        }
        else
        {
            if ( !digitsOnly.isEmpty() )
            {
                return digitsOnly;
            }
        }
    }

    return digitsOnly;
}
