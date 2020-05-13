/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RiaDefines.h"
#include "RiaPorosityModel.h"

#include "RigEclipseResultAddress.h"

#include "cvfCollection.h"

#include <QDateTime>

#include <cmath>
#include <map>
#include <vector>

class RifReaderInterface;
class RigActiveCellInfo;
class RigMainGrid;
class RigEclipseResultInfo;
class RigStatisticsDataCache;
class RigEclipseTimeStepInfo;
class RigEclipseCaseData;
class RigFormationNames;
class RigAllanDiagramData;

class RimEclipseCase;

//==================================================================================================
/// Class containing the results for the complete number of active cells. Both main grid and LGR's
//==================================================================================================
class RigCaseCellResultsData : public cvf::Object
{
public:
    explicit RigCaseCellResultsData( RigEclipseCaseData* ownerCaseData, RiaDefines::PorosityModelType porosityModel );

    // Initialization

    void                      setReaderInterface( RifReaderInterface* readerInterface );
    const RifReaderInterface* readerInterface() const;
    void                      setHdf5Filename( const QString& hdf5SourSimFilename );
    void                      setActiveFormationNames( RigFormationNames* activeFormationNames );
    const RigFormationNames*  activeFormationNames() const;
    RigAllanDiagramData*      allanDiagramData();

    void                     setMainGrid( RigMainGrid* ownerGrid );
    void                     setActiveCellInfo( RigActiveCellInfo* activeCellInfo );
    RigActiveCellInfo*       activeCellInfo();
    const RigActiveCellInfo* activeCellInfo() const;

    // Access the results data

    const std::vector<std::vector<double>>& cellScalarResults( const RigEclipseResultAddress& resVarAddr ) const;
    const std::vector<double>& cellScalarResults( const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex ) const;
    std::vector<std::vector<double>>* modifiableCellScalarResultTimesteps( const RigEclipseResultAddress& resVarAddr );
    std::vector<double>* modifiableCellScalarResult( const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex );

    bool isUsingGlobalActiveIndex( const RigEclipseResultAddress& resVarAddr ) const;

    static const std::vector<double>* getResultIndexableStaticResult( RigActiveCellInfo*      actCellInfo,
                                                                      RigCaseCellResultsData* gridCellResults,
                                                                      QString                 porvResultName,
                                                                      std::vector<double>& activeCellsResultsTempContainer );
    // Statistic values of the results

    void recalculateStatistics( const RigEclipseResultAddress& resVarAddr );
    void minMaxCellScalarValues( const RigEclipseResultAddress& resVarAddr, double& min, double& max );
    void minMaxCellScalarValues( const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex, double& min, double& max );
    void posNegClosestToZero( const RigEclipseResultAddress& resVarAddr, double& pos, double& neg );
    void posNegClosestToZero( const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex, double& pos, double& neg );
    const std::vector<size_t>& cellScalarValuesHistogram( const RigEclipseResultAddress& resVarAddr );
    const std::vector<size_t>& cellScalarValuesHistogram( const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex );
    void p10p90CellScalarValues( const RigEclipseResultAddress& resVarAddr, double& p10, double& p90 );
    void p10p90CellScalarValues( const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex, double& p10, double& p90 );
    void meanCellScalarValues( const RigEclipseResultAddress& resVarAddr, double& meanValue );
    void meanCellScalarValues( const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex, double& meanValue );
    const std::vector<int>& uniqueCellScalarValues( const RigEclipseResultAddress& resVarAddr );
    void                    sumCellScalarValues( const RigEclipseResultAddress& resVarAddr, double& sumValue );
    void sumCellScalarValues( const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex, double& sumValue );
    void mobileVolumeWeightedMean( const RigEclipseResultAddress& resVarAddr, double& meanValue );
    void mobileVolumeWeightedMean( const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex, double& meanValue );

    // Access meta-information about the results

    size_t timeStepCount( const RigEclipseResultAddress& resVarAddr ) const;
    size_t maxTimeStepCount( RigEclipseResultAddress* resultAddressWithMostTimeSteps = nullptr ) const;

    std::vector<QDateTime> allTimeStepDatesFromEclipseReader() const;
    std::vector<QDateTime> timeStepDates() const;
    std::vector<QDateTime> timeStepDates( const RigEclipseResultAddress& resVarAddr ) const;
    std::vector<double>    daysSinceSimulationStart() const;
    std::vector<double>    daysSinceSimulationStart( const RigEclipseResultAddress& resVarAddr ) const;
    int                    reportStepNumber( const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex ) const;

    std::vector<RigEclipseTimeStepInfo> timeStepInfos( const RigEclipseResultAddress& resVarAddr ) const;
    void                                setTimeStepInfos( const RigEclipseResultAddress&             resVarAddr,
                                                          const std::vector<RigEclipseTimeStepInfo>& timeStepInfos );

    void clearScalarResult( RiaDefines::ResultCatType type, const QString& resultName );
    void clearScalarResult( const RigEclipseResultAddress& resultAddress );
    void clearAllResults();
    void freeAllocatedResultsData();
    void eraseAllSourSimData();

    QStringList                          resultNames( RiaDefines::ResultCatType type ) const;
    std::vector<RigEclipseResultAddress> existingResults() const;
    const RigEclipseResultInfo*          resultInfo( const RigEclipseResultAddress& resVarAddr ) const;
    bool    updateResultName( RiaDefines::ResultCatType resultType, const QString& oldName, const QString& newName );
    QString makeResultNameUnique( const QString& resultNameProposal ) const;

    void ensureKnownResultLoadedForTimeStep( const RigEclipseResultAddress& resultAddress, size_t timeStepIndex );
    bool ensureKnownResultLoaded( const RigEclipseResultAddress& resultAddress );

    bool findAndLoadResultByName( const QString&                                resultName,
                                  const std::vector<RiaDefines::ResultCatType>& resultCategorySearchOrder );

    bool hasResultEntry( const RigEclipseResultAddress& resultAddress ) const;
    bool isResultLoaded( const RigEclipseResultAddress& resultAddress ) const;
    void createResultEntry( const RigEclipseResultAddress& resultAddress, bool needsToBeStored );
    void createPlaceholderResultEntries();
    void computeDepthRelatedResults();
    void computeCellVolumes();
    bool hasFlowDiagUsableFluxes() const;

    static void copyResultsMetaDataFromMainCase( RigEclipseCaseData*           mainCaseResultsData,
                                                 RiaDefines::PorosityModelType poroModel,
                                                 std::vector<RimEclipseCase*>  destinationCases );

private:
    size_t findOrLoadKnownScalarResult( const RigEclipseResultAddress& resVarAddr );
    size_t findOrLoadKnownScalarResultByResultTypeOrder( const RigEclipseResultAddress&                resVarAddr,
                                                         const std::vector<RiaDefines::ResultCatType>& resultCategorySearchOrder );

    size_t findOrLoadKnownScalarResultForTimeStep( const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex );
    size_t findOrCreateScalarResultIndex( const RigEclipseResultAddress& resVarAddr, bool needsToBeStored );

    size_t findScalarResultIndexFromAddress( const RigEclipseResultAddress& resVarAddr ) const;

    size_t addStaticScalarResult( RiaDefines::ResultCatType type,
                                  const QString&            resultName,
                                  bool                      needsToBeStored,
                                  size_t                    resultValueCount );

    const std::vector<RigEclipseResultInfo>& infoForEachResultIndex();
    size_t                                   resultCount() const;

    bool mustBeCalculated( size_t scalarResultIndex ) const;
    void setMustBeCalculated( size_t scalarResultIndex );

    void computeSOILForTimeStep( size_t timeStepIndex );
    void testAndComputeSgasForTimeStep( size_t timeStepIndex );

    bool hasCompleteTransmissibilityResults() const;

    void computeRiTransComponent( const QString& riTransComponentResultName );
    void computeNncCombRiTrans();

    void computeRiMULTComponent( const QString& riMultCompName );
    void computeNncCombRiMULT();
    void computeRiTRANSbyAreaComponent( const QString& riTransByAreaCompResultName );
    void computeNncCombRiTRANSbyArea();

    void   computeCompletionTypeForTimeStep( size_t timeStep );
    double darchysValue();

    void computeOilVolumes();
    void computeMobilePV();

    bool isDataPresent( size_t scalarResultIndex ) const;

    void assignValuesToTemporaryLgrs( const QString& resultName, std::vector<double>& values );

    RigStatisticsDataCache* statistics( const RigEclipseResultAddress& resVarAddr );

    static void
        computeAllanResults( RigCaseCellResultsData* cellResultsData, RigMainGrid* mainGrid, bool includeInactiveCells );

private:
    cvf::ref<RifReaderInterface>  m_readerInterface;
    cvf::cref<RigFormationNames>  m_activeFormationNamesData;
    cvf::ref<RigAllanDiagramData> m_allanDiagramData;

    std::vector<std::vector<std::vector<double>>> m_cellScalarResults; ///< Scalar results on the complete reservoir for
                                                                       ///< each Result index (ResultVariable) and timestep
    cvf::Collection<RigStatisticsDataCache>   m_statisticsDataCache;
    std::vector<RigEclipseResultInfo>         m_resultInfos;
    std::map<RigEclipseResultAddress, size_t> m_addressToResultIndexMap;

    RigMainGrid*                  m_ownerMainGrid;
    RigEclipseCaseData*           m_ownerCaseData;
    RigActiveCellInfo*            m_activeCellInfo;
    RiaDefines::PorosityModelType m_porosityModel;
};
