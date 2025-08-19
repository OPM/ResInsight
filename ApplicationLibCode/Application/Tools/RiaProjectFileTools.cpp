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
#include "Tools/RiaVariableMapper.h"

#include "RimCase.h"
#include "RimCaseDisplayNameTools.h"
#include "RimProject.h"
#include "RimSummaryCase.h"

#include <QStringList>

namespace internal
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool isCandidateNewerThanOther( int candidateMajorVersion,
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

    // Early exit if a minor number is undefined
    if ( candidateMinorVersion == -1 || otherMinorVersion == -1 ) return false;

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
QString stringOfDigits( const QString& string )
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

} // namespace internal

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaProjectFileTools::isCandidateVersionNewerThanOther( const QString& candidateProjectFileVersion, const QString& projectFileVersion )
{
    int candidateMajorVersion  = 0;
    int candidateMinorVersion  = -1;
    int candidatePatchNumber   = -1;
    int candidateDevelopmentId = -1;

    RiaProjectFileTools::decodeVersionString( candidateProjectFileVersion,
                                              &candidateMajorVersion,
                                              &candidateMinorVersion,
                                              &candidatePatchNumber,
                                              &candidateDevelopmentId );

    int majorVersion  = 0;
    int minorVersion  = -1;
    int patchNumber   = -1;
    int developmentId = -1;

    RiaProjectFileTools::decodeVersionString( projectFileVersion, &majorVersion, &minorVersion, &patchNumber, &developmentId );

    return internal::isCandidateNewerThanOther( candidateMajorVersion,
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
QString RiaProjectFileTools::transferPathsToGlobalPathList( RimProject* project )
{
    RiaVariableMapper variableMapper( "" );

    for ( caf::FilePath* filePath : project->allFilePaths() )
    {
        QString path = filePath->path();
        if ( !path.isEmpty() )
        {
            QString pathId = variableMapper.addPathAndGetId( path );
            filePath->setPath( pathId );
        }
    }

    for ( auto summaryCase : project->allSummaryCases() )
    {
        if ( summaryCase->displayNameType() == RimCaseDisplayNameTools::DisplayName::CUSTOM )
        {
            // At this point, after the replace of variables into caf::FilePath objects, the variable name is
            // stored in the summary case object. Read out the variable name and append "_name" for custom
            // summary variables.

            QString variableName = summaryCase->summaryHeaderFilename();
            if ( !variableName.isEmpty() )
            {
                variableName = variableName.remove( RiaVariableMapper::variableToken() );

                variableName = RiaVariableMapper::variableToken() + variableName + RiaVariableMapper::postfixName() +
                               RiaVariableMapper::variableToken();

                QString variableValue = summaryCase->displayCaseName();
                variableMapper.addVariable( variableName, variableValue );

                summaryCase->setCustomCaseName( variableName );
            }
        }
    }

    for ( auto gridCase : project->allGridCases() )
    {
        if ( gridCase->displayNameType() == RimCaseDisplayNameTools::DisplayName::CUSTOM )
        {
            // At this point, after the replace of variables into caf::FilePath objects, the variable name is
            // stored in the summary case object. Read out the variable name and append "_name" for custom
            // summary variables.

            QString variableName = gridCase->gridFileName();
            variableName         = variableName.remove( RiaVariableMapper::variableToken() );

            variableName = RiaVariableMapper::variableToken() + variableName + RiaVariableMapper::postfixName() +
                           RiaVariableMapper::variableToken();

            QString variableValue = gridCase->caseUserDescription();
            variableMapper.addVariable( variableName, variableValue );

            gridCase->setCustomCaseName( variableName );
        }
    }

    variableMapper.replaceVariablesInValues();

    return variableMapper.variableTableAsText();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaProjectFileTools::distributePathsFromGlobalPathList( RimProject* project, const QString& pathList )
{
    if ( !project ) return;

    RiaVariableMapper pathListMapper( pathList );

    for ( auto filePath : project->allFilePaths() )
    {
        filePath->setPath( updatedFilePathFromPathId( filePath->path(), &pathListMapper ) );
    }

    for ( auto summaryCase : project->allSummaryCases() )
    {
        if ( summaryCase->displayNameType() == RimCaseDisplayNameTools::DisplayName::CUSTOM )
        {
            auto variableName = summaryCase->displayCaseName();

            bool    isFound       = false;
            QString variableValue = pathListMapper.valueForVariable( variableName, &isFound );
            if ( isFound )
            {
                summaryCase->setCustomCaseName( variableValue );
            }
            else if ( variableName.contains( RiaVariableMapper::postfixName() + RiaVariableMapper::variableToken() ) )
            {
                // The variable name is not found in the variable list, but the name indicates a variable. Reset
                // to full case name.
                summaryCase->setDisplayNameOption( RimCaseDisplayNameTools::DisplayName::FULL_CASE_NAME );
            }
        }
    }

    for ( auto gridCase : project->allGridCases() )
    {
        if ( gridCase->displayNameType() == RimCaseDisplayNameTools::DisplayName::CUSTOM )
        {
            auto variableName = gridCase->caseUserDescription();

            bool    isFound       = false;
            QString variableValue = pathListMapper.valueForVariable( variableName, &isFound );
            if ( isFound )
            {
                gridCase->setCustomCaseName( variableValue );
            }
            else if ( variableName.contains( RiaVariableMapper::postfixName() + RiaVariableMapper::variableToken() ) )
            {
                // The variable name is not found in the variable list, but the name indicates a variable. Reset
                // to full case name.
                gridCase->setDisplayNameType( RimCaseDisplayNameTools::DisplayName::FULL_CASE_NAME );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaProjectFileTools::updatedFilePathFromPathId( QString filePath, RiaVariableMapper* pathListMapper )
{
    if ( pathListMapper )
    {
        QString     pathIdCandidate  = filePath.trimmed();
        QStringList pathIdComponents = pathIdCandidate.split( RiaVariableMapper::variableToken() );

        if ( pathIdComponents.size() == 3 && pathIdComponents[0].size() == 0 && pathIdComponents[1].size() > 0 &&
             pathIdComponents[2].size() == 0 )
        {
            bool    isFound = false;
            QString path    = pathListMapper->valueForVariable( pathIdCandidate, &isFound );
            if ( isFound )
            {
                return path;
            }
        }
    }

    return filePath;
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
        QString candidateDigitsOnly = internal::stringOfDigits( candidate );

        *patch = candidateDigitsOnly.toInt();
    }

    if ( subStrings.size() > 3 )
    {
        QString candidate           = subStrings.back();
        QString candidateDigitsOnly = internal::stringOfDigits( candidate );

        *developmentId = candidateDigitsOnly.toInt();
    }
}
