/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "RimOpmFlowJobTools.h"

#include "Hpc/RiaHpcTools.h"
#include "RiaLogging.h"
#include "RiaPreferencesOpm.h"
#include "RiaWslTools.h"

#include "CompletionExportCommands/RicExportCompletionDataSettingsUi.h"
#include "CompletionExportCommands/RicWellPathExportCompletionDataFeatureImpl.h"
#include "CompletionExportCommands/RicWellPathExportMswTableData.h"

#include "RifOpmFlowDeckFile.h"

#include "CompletionsMsw/RigMswTableData.h"

#include "RimEclipseCase.h"
#include "RimKeywordFactory.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCompletionSettings.h"

namespace RimOpmFlowJobTools
{

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* findExistingCase( QString filename )
{
    if ( RimProject* proj = RimProject::current() )
    {
        for ( RimCase* c : proj->allGridCases() )
        {
            RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( c );
            if ( eclipseCase && ( eclipseCase->gridFileName() == filename ) )
            {
                return eclipseCase;
            }
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int mergeMswData( RifOpmFlowDeckFile*              deckFile,
                  int                              mergeDeckPosition,
                  RimEclipseCase*                  eclipseCase,
                  RimWellPath*                     wellPath,
                  RimJobWellSettings::WellOpenType wellOpenType,
                  int                              openTimeStep )

{
    const int failure = -1;

    auto exportDate    = RicWellPathExportCompletionDataFeatureImpl::exportDateForTimeStep( eclipseCase, 0 );
    auto mswDataResult = RicWellPathExportMswTableData::extractSingleWellMswData( eclipseCase,
                                                                                  wellPath,
                                                                                  true,
                                                                                  RicWellPathExportMswTableData::CompletionType::ALL,
                                                                                  exportDate );
    if ( !mswDataResult.has_value() )
    {
        RiaLogging::error( mswDataResult.error() );
        return failure;
    }

    int  maxSegNum   = 0;
    int  maxBranches = 0;
    auto welsegsKw   = RimKeywordFactory::welsegsKeyword( mswDataResult.value(), maxSegNum, maxBranches );
    auto compsegsKw  = RimKeywordFactory::compsegsKeyword( mswDataResult.value() );
    auto wsegvalvKw  = RimKeywordFactory::wsegvalvKeyword( mswDataResult.value() );
    auto wsegaicdKw  = RimKeywordFactory::wsegaicdKeyword( mswDataResult.value() );
    auto wsegsicdKw  = RimKeywordFactory::wsegsicdKeyword( mswDataResult.value() );

    if ( compsegsKw.empty() )
    {
        RiaLogging::warning( "Unable to create COMPSEGS keyword from MSW data, skipping MSW export. Is the wellpath outside the grid?" );
        return ( wellOpenType == RimJobWellSettings::WellOpenType::OPEN_AT_DATE ) ? 0 : mergeDeckPosition;
    }

    if ( welsegsKw.empty() )
    {
        RiaLogging::warning( "Unable to create WELSEGS keyword from MSW data. Is the wellpath outside the grid?" );
    }

    if ( wellOpenType == RimJobWellSettings::WellOpenType::OPEN_AT_DATE )
    {
        // make sure we insert after COMPDAT kw
        if ( !deckFile->addKeywordAtTimeStep( openTimeStep, welsegsKw, "COMPDAT" ) ) return failure;
        if ( !deckFile->addKeywordAtTimeStep( openTimeStep, compsegsKw, welsegsKw.name() ) ) return failure;
        if ( !wsegvalvKw.empty() )
        {
            if ( !deckFile->mergeKeywordAtTimeStep( openTimeStep, wsegvalvKw, compsegsKw.name() ) ) return failure;
        }
        if ( !wsegaicdKw.empty() )
        {
            if ( !deckFile->mergeKeywordAtTimeStep( openTimeStep, wsegaicdKw, compsegsKw.name() ) ) return failure;
        }
        if ( !wsegsicdKw.empty() )
        {
            if ( !deckFile->mergeKeywordAtTimeStep( openTimeStep, wsegsicdKw, compsegsKw.name() ) ) return failure;
        }

        mergeDeckPosition = 0;
    }
    else
    {
        mergeDeckPosition = deckFile->addKeywordAtPosition( mergeDeckPosition, welsegsKw );
        if ( mergeDeckPosition < 0 ) return failure;
        mergeDeckPosition++;
        mergeDeckPosition = deckFile->addKeywordAtPosition( mergeDeckPosition, compsegsKw );
        if ( mergeDeckPosition < 0 ) return failure;
        mergeDeckPosition++;
        if ( !wsegvalvKw.empty() )
        {
            mergeDeckPosition = deckFile->mergeKeywordAtPosition( mergeDeckPosition, wsegvalvKw );
            if ( mergeDeckPosition < 0 ) return failure;
            mergeDeckPosition++;
        }
        if ( !wsegaicdKw.empty() )
        {
            mergeDeckPosition = deckFile->mergeKeywordAtPosition( mergeDeckPosition, wsegaicdKw );
            if ( mergeDeckPosition < 0 ) return failure;
            mergeDeckPosition++;
        }
        if ( !wsegsicdKw.empty() )
        {
            mergeDeckPosition = deckFile->mergeKeywordAtPosition( mergeDeckPosition, wsegsicdKw );
            if ( mergeDeckPosition < 0 ) return failure;
            mergeDeckPosition++;
        }
    }

    int branches = (int)wellPath->allWellPathLaterals().size() + 1;

    // wsegdims contains: max segmented wells, max segments, max branches
    auto wsegdims    = deckFile->wsegdims();
    int  maxSegWells = wsegdims[0] + 1; // we have added one well
    maxSegNum        = std::max( wsegdims[1], maxSegNum );
    maxBranches      = std::max( branches, maxBranches );
    maxBranches      = std::max( wsegdims[2], maxBranches );
    if ( !deckFile->setWsegdims( maxSegWells, maxSegNum, maxBranches ) )
    {
        RiaLogging::error( "Failed to update WSEGDIMS keyword in DATA file." );
        return failure;
    }
    return mergeDeckPosition;
}

//--------------------------------------------------------------------------------------------------
///  Returns value < 0 if failure to merge
//--------------------------------------------------------------------------------------------------
int mergeBasicWellSettings( RifOpmFlowDeckFile*              deckFile,
                            int                              mergeDeckPosition,
                            RimEclipseCase*                  eclipseCase,
                            RimWellPath*                     wellPath,
                            RimJobWellSettings::WellOpenType wellOpenType,
                            int                              openTimeStep,
                            QString                          wellGroupName )
{
    const int failure = -1;

    if ( eclipseCase == nullptr || wellPath == nullptr || wellPath->completionSettings() == nullptr || eclipseCase->eclipseCaseData() == nullptr )
    {
        RiaLogging::error( "Cannot create COMPDAT keyword - check case, wellpath and completion settings data." );
        return failure;
    }

    auto compdata = RicWellPathExportCompletionDataFeatureImpl::completionDataForWellPath( wellPath, eclipseCase );
    auto wellName = wellPath->completionSettings()->wellNameForExport().toStdString();

    int mergePosition = mergeDeckPosition;

    auto compdatKw  = RimKeywordFactory::compdatKeyword( compdata, wellName );
    auto wpimultKw  = RimKeywordFactory::wpimultKeyword( compdata, wellName );
    auto complumpKw = RimKeywordFactory::complumpKeyword( compdata, wellName );
    auto welspecsKw = RimKeywordFactory::welspecsKeyword( wellGroupName.toStdString(), eclipseCase, wellPath );

    if ( compdatKw.empty() )
    {
        RiaLogging::warning( "Failed to create COMPDAT keyword for selected well path. Do you have a valid case selected?" );
    }

    if ( welspecsKw.empty() )
    {
        RiaLogging::error( "Failed to create WELSPECS keyword for selected well path. Do you have a valid case selected?" );
        return failure;
    }

    if ( wellOpenType == RimJobWellSettings::WellOpenType::OPEN_AT_DATE )
    {
        // reverse order for correct insertion order
        if ( !complumpKw.empty() )
        {
            if ( !deckFile->mergeKeywordAtTimeStep( openTimeStep, complumpKw ) ) return failure;
        }
        if ( !wpimultKw.empty() )
        {
            if ( !deckFile->mergeKeywordAtTimeStep( openTimeStep, wpimultKw ) ) return failure;
        }
        if ( !deckFile->mergeKeywordAtTimeStep( openTimeStep, compdatKw ) ) return failure;
        if ( !deckFile->mergeKeywordAtTimeStep( openTimeStep, welspecsKw ) ) return failure;
        mergePosition = 0;
    }
    else
    {
        mergePosition = deckFile->mergeKeywordAtPosition( mergePosition, welspecsKw );
        if ( mergePosition < 0 ) return failure;
        mergePosition = deckFile->mergeKeywordAtPosition( mergePosition, compdatKw );
        if ( mergePosition < 0 ) return failure;
        if ( !wpimultKw.empty() )
        {
            mergePosition = deckFile->mergeKeywordAtPosition( mergePosition, wpimultKw );
            if ( mergePosition < 0 ) return failure;
        }
        if ( !complumpKw.empty() )
        {
            mergePosition = deckFile->mergeKeywordAtPosition( mergePosition, complumpKw );
            if ( mergePosition < 0 ) return failure;
        }
    }

    // increase wells and connections in welldims to make sure they are big enough
    auto additionalConnections = (int)compdatKw.size();
    auto welldims              = deckFile->welldims();
    if ( ( welldims.size() < 4 ) ||
         !deckFile->setWelldims( (int)welldims[0] + 1, (int)( welldims[1] + additionalConnections ), (int)welldims[2] + 1, (int)welldims[3] + 1 ) )
    {
        RiaLogging::error( "Failed to update WELLDIMS keyword in DATA file, is it missing?" );
        return failure;
    }
    return mergePosition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool runMakeFlux( QString parentDeckFile, QString fluxnumFile, QString outputFileName )
{
    auto prefs = RiaPreferencesOpm::current();

    QString parent  = parentDeckFile;
    QString fluxnum = fluxnumFile;
    QString output  = outputFileName;

    if ( prefs->useWsl() )
    {
        parent  = RiaWslTools::convertToWslPath( parent );
        fluxnum = RiaWslTools::convertToWslPath( fluxnum );
        output  = RiaWslTools::convertToWslPath( output );
    }

    QStringList arguments;
    arguments << "--parent=" + parent;
    arguments << "--fluxnum=" + fluxnum;
    arguments << "--regions=1";
    arguments << "--output=" + output;

    auto [ok, stdOut] = RiaHpcTools::runUtilityCommand( prefs->makeFluxCommand(), arguments, prefs->wslOptions() );

    if ( stdOut.size() > 0 )
    {
        RiaLogging::debug( "make_flux output:" );
        for ( auto& line : stdOut )
        {
            RiaLogging::debug( line.toStdString() );
        }
    }

    return ok;
}

} // namespace RimOpmFlowJobTools