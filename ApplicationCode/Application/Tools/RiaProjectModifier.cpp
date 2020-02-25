/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "RiaProjectModifier.h"

#include "RimCaseCollection.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseInputCase.h"
#include "RimEclipseResultCase.h"
#include "RimGeoMechCase.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimOilField.h"
#include "RimProject.h"

#include <QDir>
#include <QFileInfo>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaProjectModifier::RiaProjectModifier()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaProjectModifier::setReplaceCaseFirstOccurrence( const QString& newGridFileName )
{
    m_caseIdToGridFileNameMap[RiaProjectModifier::firstOccurrenceId()] = makeFilePathAbsolute( newGridFileName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaProjectModifier::setReplaceCase( int caseIdToReplace, const QString& newGridFileName )
{
    if ( caseIdToReplace >= 0 )
    {
        m_caseIdToGridFileNameMap[caseIdToReplace] = makeFilePathAbsolute( newGridFileName );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaProjectModifier::setReplaceSourceCasesFirstOccurrence( const std::vector<QString>& newGridFileNames )
{
    m_groupIdToGridFileNamesMap[RiaProjectModifier::firstOccurrenceId()] = newGridFileNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaProjectModifier::setReplaceSourceCasesById( int caseGroupIdToReplace, const std::vector<QString>& newGridFileNames )
{
    if ( caseGroupIdToReplace >= 0 )
    {
        m_groupIdToGridFileNamesMap[caseGroupIdToReplace] = newGridFileNames;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaProjectModifier::setReplacePropertiesFolderFirstOccurrence( QString newPropertiesFolder )
{
    m_caseIdToPropertiesFolderMap[RiaProjectModifier::firstOccurrenceId()] = makeFilePathAbsolute( newPropertiesFolder );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaProjectModifier::setReplacePropertiesFolder( int caseIdToReplace, QString newPropertiesFolder )
{
    if ( caseIdToReplace >= 0 )
    {
        m_caseIdToPropertiesFolderMap[caseIdToReplace] = makeFilePathAbsolute( newPropertiesFolder );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaProjectModifier::applyModificationsToProject( RimProject* project )
{
    if ( m_caseIdToGridFileNameMap.size() > 0 )
    {
        replaceCase( project );
    }

    if ( m_groupIdToGridFileNamesMap.size() > 0 )
    {
        replaceSourceCases( project );
    }

    if ( m_caseIdToPropertiesFolderMap.size() > 0 )
    {
        replacePropertiesFolder( project );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaProjectModifier::replaceSourceCases( RimProject* project )
{
    for ( RimOilField* oilField : project->oilFields() )
    {
        RimEclipseCaseCollection* analysisModels = oilField ? oilField->analysisModels() : nullptr;
        if ( analysisModels )
        {
            for ( RimIdenticalGridCaseGroup* caseGroup : analysisModels->caseGroups )
            {
                for ( auto item : m_groupIdToGridFileNamesMap )
                {
                    int groupIdToReplace = item.first;
                    if ( groupIdToReplace == RiaProjectModifier::firstOccurrenceId() )
                    {
                        groupIdToReplace = firstGroupId( project );
                    }

                    if ( groupIdToReplace == caseGroup->groupId() )
                    {
                        RimCaseCollection* caseCollection = caseGroup->caseCollection;
                        caseCollection->reservoirs.deleteAllChildObjects();

                        for ( QString fileName : item.second )
                        {
                            QString caseName = caseNameFromGridFileName( fileName );

                            // Use this slightly hackish method in order to get a new unique ID
                            RimEclipseResultCase* resCase = new RimEclipseResultCase;
                            resCase->setCaseInfo( caseName, fileName );

                            caseCollection->reservoirs.push_back( resCase );
                        }
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaProjectModifier::replaceCase( RimProject* project )
{
    std::vector<RimCase*> allCases;
    project->allCases( allCases );

    for ( RimCase* rimCase : allCases )
    {
        RimEclipseResultCase* eclipseResultCase = dynamic_cast<RimEclipseResultCase*>( rimCase );
        RimGeoMechCase*       geomechCase       = dynamic_cast<RimGeoMechCase*>( rimCase );
        if ( eclipseResultCase || geomechCase )
        {
            for ( auto item : m_caseIdToGridFileNameMap )
            {
                int caseIdToReplace = item.first;
                if ( caseIdToReplace == RiaProjectModifier::firstOccurrenceId() )
                {
                    caseIdToReplace = firstCaseId( project );
                }

                if ( caseIdToReplace == rimCase->caseId() )
                {
                    QString replaceFileName = item.second;
                    if ( eclipseResultCase )
                    {
                        eclipseResultCase->setGridFileName( replaceFileName );
                        eclipseResultCase->caseUserDescription = caseNameFromGridFileName( replaceFileName );
                    }
                    else if ( geomechCase )
                    {
                        geomechCase->setGridFileName( replaceFileName );
                        geomechCase->caseUserDescription = caseNameFromGridFileName( replaceFileName );
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaProjectModifier::replacePropertiesFolder( RimProject* project )
{
    std::vector<RimCase*> allCases;
    project->allCases( allCases );

    for ( RimCase* rimCase : allCases )
    {
        RimEclipseInputCase* inputCase = dynamic_cast<RimEclipseInputCase*>( rimCase );

        if ( inputCase )
        {
            for ( auto item : m_caseIdToPropertiesFolderMap )
            {
                int caseIdToReplace = item.first;

                if ( caseIdToReplace == RiaProjectModifier::firstOccurrenceId() )
                {
                    caseIdToReplace = firstInputCaseId( project );
                }

                if ( caseIdToReplace == inputCase->caseId() )
                {
                    inputCase->updateAdditionalFileFolder( item.second );
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Returns absolute path name to the specified file.
///
/// If \a relOrAbsolutePath is a relative, the current working directory for the process will be
/// used in order to make the path absolute.
//--------------------------------------------------------------------------------------------------
QString RiaProjectModifier::makeFilePathAbsolute( const QString& relOrAbsolutePath )
{
    QFileInfo theFile( relOrAbsolutePath );
    theFile.makeAbsolute();
    return theFile.filePath();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaProjectModifier::caseNameFromGridFileName( const QString& fullGridFilePathName )
{
    QString fn = QDir::fromNativeSeparators( fullGridFilePathName );

    // Extract file name plus the 'deepest' directory
    QString deepestDirPlusFileName = fn.section( '/', -2, -1 );

    deepestDirPlusFileName.replace( "/", "--" );

    return deepestDirPlusFileName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaProjectModifier::firstCaseId( RimProject* project )
{
    std::vector<RimCase*> allCases;
    project->allCases( allCases );

    for ( RimCase* rimCase : allCases )
    {
        RimEclipseResultCase* resultCase = dynamic_cast<RimEclipseResultCase*>( rimCase );
        if ( resultCase )
        {
            return resultCase->caseId();
        }
    }

    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaProjectModifier::firstGroupId( RimProject* project )
{
    for ( size_t oilFieldIdx = 0; oilFieldIdx < project->oilFields().size(); oilFieldIdx++ )
    {
        RimOilField*              oilField       = project->oilFields[oilFieldIdx];
        RimEclipseCaseCollection* analysisModels = oilField ? oilField->analysisModels() : nullptr;
        if ( analysisModels )
        {
            if ( analysisModels->caseGroups.size() > 0 )
            {
                return analysisModels->caseGroups[0]->groupId();
            }
        }
    }

    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaProjectModifier::firstInputCaseId( RimProject* project )
{
    std::vector<RimCase*> allCases;
    project->allCases( allCases );

    for ( RimCase* rimCase : allCases )
    {
        RimEclipseInputCase* resultCase = dynamic_cast<RimEclipseInputCase*>( rimCase );
        if ( resultCase )
        {
            return resultCase->caseId();
        }
    }

    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaProjectModifier::firstOccurrenceId()
{
    return -999;
}
