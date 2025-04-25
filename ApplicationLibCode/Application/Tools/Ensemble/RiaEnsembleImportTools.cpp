/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RiaEnsembleImportTools.h"

#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "Summary/RiaSummaryTools.h"

#include "RifSummaryCaseRestartSelector.h"

#include "RimSummaryCaseMainCollection.h"

namespace RiaEnsembleImportTools
{

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, std::vector<RimSummaryCase*>> createSummaryCasesFromFiles( const QStringList& fileNames, CreateConfig createConfig )
{
    RimSummaryCaseMainCollection* sumCaseColl = RiaSummaryTools::summaryCaseMainCollection();

    std::vector<RimSummaryCase*> newCases;
    if ( !sumCaseColl ) return std::make_pair( false, newCases );

    std::vector<RifSummaryCaseFileResultInfo> importFileInfos;
    if ( createConfig.fileType == RiaDefines::FileType::SMSPEC )
    {
        RifSummaryCaseRestartSelector fileSelector;

        if ( !RiaGuiApplication::isRunning() || !createConfig.allowDialogs )
        {
            fileSelector.showDialog( false );
        }

        fileSelector.setEnsembleOrGroupMode( createConfig.ensembleOrGroup );
        fileSelector.determineFilesToImportFromSummaryFiles( fileNames );

        importFileInfos = fileSelector.summaryFileInfos();

        if ( fileSelector.foundErrors() )
        {
            QString errorMessage = fileSelector.createCombinedErrorMessage();
            RiaLogging::error( errorMessage );
        }
    }
    else
    {
        // No restart files for these file types: just copy to result info
        for ( const auto& f : fileNames )
        {
            importFileInfos.push_back( RifSummaryCaseFileResultInfo( f, false, createConfig.fileType ) );
        }
    }

    if ( !importFileInfos.empty() )
    {
        std::vector<RimSummaryCase*> sumCases = sumCaseColl->createSummaryCasesFromFileInfos( importFileInfos, true );
        newCases.insert( newCases.end(), sumCases.begin(), sumCases.end() );
    }

    return std::make_pair( true, newCases );
}

} // namespace RiaEnsembleImportTools
