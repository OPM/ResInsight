/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023-     Equinor ASA
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

#include "RigAllanUtil.h"
#include "RigAllanDiagramData.h"
#include "RigCaseCellResultsData.h"
#include "RigFormationNames.h"
#include "RigMainGrid.h"
#include "RigNNCData.h"
#include "RigNncConnection.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigAllanUtil::computeAllanResults( RigCaseCellResultsData* cellResultsData, RigMainGrid* mainGrid, bool includeInactiveCells )
{
    CVF_ASSERT( mainGrid );
    CVF_ASSERT( cellResultsData );

    if ( cellResultsData && cellResultsData->activeFormationNames() && !cellResultsData->activeFormationNames()->formationNames().empty() )
    {
        auto fnNamesResAddr =
            RigEclipseResultAddress( RiaDefines::ResultCatType::FORMATION_NAMES, RiaResultNames::activeFormationNamesResultName() );
        auto fnAllanResultResAddr =
            RigEclipseResultAddress( RiaDefines::ResultCatType::ALLAN_DIAGRAMS, RiaResultNames::formationAllanResultName() );
        auto fnBinAllanResAddr =
            RigEclipseResultAddress( RiaDefines::ResultCatType::ALLAN_DIAGRAMS, RiaResultNames::formationBinaryAllanResultName() );

        // Create and retrieve nnc result arrays

        std::vector<double>& fnAllanNncResults =
            mainGrid->nncData()->makeStaticConnectionScalarResult( RiaResultNames::formationAllanResultName() );
        std::vector<double>& fnBinAllanNncResults =
            mainGrid->nncData()->makeStaticConnectionScalarResult( RiaResultNames::formationBinaryAllanResultName() );

        // Associate them with eclipse result address

        mainGrid->nncData()->setEclResultAddress( RiaResultNames::formationAllanResultName(), fnAllanResultResAddr );
        mainGrid->nncData()->setEclResultAddress( RiaResultNames::formationBinaryAllanResultName(), fnBinAllanResAddr );

        const std::vector<double>& fnData = cellResultsData->cellScalarResults( fnNamesResAddr, 0 );

        // Add a result entry for the special Allan grid data (used only for the grid cells without nnc coverage)

        cellResultsData->addStaticScalarResult( RiaDefines::ResultCatType::ALLAN_DIAGRAMS,
                                                RiaResultNames::formationAllanResultName(),
                                                false,
                                                fnData.size() );
        cellResultsData->addStaticScalarResult( RiaDefines::ResultCatType::ALLAN_DIAGRAMS,
                                                RiaResultNames::formationBinaryAllanResultName(),
                                                false,
                                                fnData.size() );

        std::vector<double>* alData    = cellResultsData->modifiableCellScalarResult( fnAllanResultResAddr, 0 );
        std::vector<double>* binAlData = cellResultsData->modifiableCellScalarResult( fnBinAllanResAddr, 0 );

        ( *alData ) = fnData;

        for ( double& val : ( *binAlData ) )
        {
            val = 0.0;
        }

        size_t formationCount = 0;
        if ( cellResultsData->activeFormationNames() )
        {
            formationCount = cellResultsData->activeFormationNames()->formationNames().size();
        }

        const RigConnectionContainer& nncConnections = mainGrid->nncData()->allConnections();

        std::map<std::pair<int, int>, int> formationCombinationToCategory;
        for ( size_t i = 0; i < nncConnections.size(); i++ )
        {
            const auto& c = nncConnections[i];

            size_t globCellIdx1 = c.c1GlobIdx();
            size_t globCellIdx2 = c.c2GlobIdx();

            int formation1 = (int)( fnData[globCellIdx1] );
            int formation2 = (int)( fnData[globCellIdx2] );

            int category = -1;
            if ( formation1 != formation2 )
            {
                if ( formation1 < formation2 )
                {
                    std::swap( formation1, formation2 );
                }

                auto formationCombination = std::make_pair( formation1, formation2 );

                auto existingCategory = formationCombinationToCategory.find( formationCombination );
                if ( existingCategory != formationCombinationToCategory.end() )
                {
                    category = existingCategory->second;
                }
                else
                {
                    category = static_cast<int>( formationCombinationToCategory.size() + formationCount );

                    formationCombinationToCategory[formationCombination] = category;
                }
                fnBinAllanNncResults[i] = 1.0;
            }
            else
            {
                category                = formation1;
                fnBinAllanNncResults[i] = 0.0;
            }

            fnAllanNncResults[i] = category;
        }

        cellResultsData->allanDiagramData()->setFormationCombinationToCategorymap( formationCombinationToCategory );
    }
    else
    {
#if 0
        for ( size_t i = 0; i < mainGrid->nncData()->connections().size(); i++ )
        {
            const auto& c = mainGrid->nncData()->connections()[i];

            size_t globCellIdx1 = c.m_c1GlobIdx;
            size_t globCellIdx2 = c.m_c2GlobIdx;

            size_t i1, j1, k1;
            mainGrid->ijkFromCellIndex( globCellIdx1, &i1, &j1, &k1 );

            size_t i2, j2, k2;
            mainGrid->ijkFromCellIndex( globCellIdx2, &i2, &j2, &k2 );

            double binaryValue = 0.0;
            if ( k1 != k2 )
            {
                binaryValue = 1.0;
            }

            fnAllanNncResults[i]        = k1;
            allAllanFormationResults[i] = k1;
            fnBinAllanNncResults[i]     = binaryValue;
        }
#endif
    }
}
