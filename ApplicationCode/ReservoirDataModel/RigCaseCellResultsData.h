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

#include "cvfCollection.h"

#include <QDateTime>

#include <vector>
#include <cmath>

class RifReaderInterface;
class RigActiveCellInfo;
class RigMainGrid;
class RigEclipseResultInfo;
class RigStatisticsDataCache;
class RigEclipseTimeStepInfo;
class RigEclipseCaseData;

class RigEclipseResultAddress
{
public:
    RigEclipseResultAddress()
        : scalarResultIndex(-1)
        , m_resultCatType(RiaDefines::UNDEFINED)
    {}

    explicit RigEclipseResultAddress(size_t ascalarResultIndex)
        : scalarResultIndex(ascalarResultIndex)
        ,  m_resultCatType(RiaDefines::UNDEFINED)
    {}

    explicit RigEclipseResultAddress(RiaDefines::ResultCatType type, const QString& resultName)
        : scalarResultIndex(-1)
        , m_resultCatType(type)
        , m_resultName(resultName)
    {}

    bool isValid() const
    {
        return (scalarResultIndex != -1);
        // Todo
    }

    size_t scalarResultIndex;

    RiaDefines::ResultCatType m_resultCatType;
    QString m_resultName;
};


//==================================================================================================
/// Class containing the results for the complete number of active cells. Both main grid and LGR's
//==================================================================================================
class RigCaseCellResultsData : public cvf::Object
{
public:
    explicit RigCaseCellResultsData(RigEclipseCaseData* ownerCaseData);

    void                                               setReaderInterface(RifReaderInterface* readerInterface);
    void                                               setHdf5Filename(const QString& hdf5SourSimFilename );

    void                                               setMainGrid(RigMainGrid* ownerGrid);
    void                                               setActiveCellInfo(RigActiveCellInfo* activeCellInfo);
    RigActiveCellInfo*                                 activeCellInfo();
    const RigActiveCellInfo*                           activeCellInfo() const;

    // Max and min values of the results
    void                                               recalculateStatistics(const RigEclipseResultAddress& resVarAddr);
    void                                               minMaxCellScalarValues(const RigEclipseResultAddress& resVarAddr, double& min, double& max);
    void                                               minMaxCellScalarValues(const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex, double& min, double& max);
    void                                               posNegClosestToZero(const RigEclipseResultAddress& resVarAddr, double& pos, double& neg);
    void                                               posNegClosestToZero(const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex, double& pos, double& neg);
    const std::vector<size_t>&                         cellScalarValuesHistogram(const RigEclipseResultAddress& resVarAddr);
    const std::vector<size_t>&                         cellScalarValuesHistogram(const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex);
    void                                               p10p90CellScalarValues(const RigEclipseResultAddress& resVarAddr, double& p10, double& p90);
    void                                               p10p90CellScalarValues(const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex, double& p10, double& p90);
    void                                               meanCellScalarValues(const RigEclipseResultAddress& resVarAddr, double& meanValue);
    void                                               meanCellScalarValues(const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex, double& meanValue);
    const std::vector<int>&                            uniqueCellScalarValues(const RigEclipseResultAddress& resVarAddr);
    void                                               sumCellScalarValues(const RigEclipseResultAddress& resVarAddr, double& sumValue);
    void                                               sumCellScalarValues(const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex, double& sumValue);
    void                                               mobileVolumeWeightedMean(const RigEclipseResultAddress& resVarAddr, double& meanValue);
    void                                               mobileVolumeWeightedMean(const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex, double& meanValue);
    // Access meta-information about the results
    size_t                                             resultCount() const;
    size_t                                             timeStepCount(const RigEclipseResultAddress& resVarAddr) const; 
    size_t                                             maxTimeStepCount(size_t* scalarResultIndex = nullptr) const; 
    QStringList                                        resultNames(RiaDefines::ResultCatType type) const;
    bool                                               isUsingGlobalActiveIndex(const RigEclipseResultAddress& resVarAddr) const;
    bool                                               hasFlowDiagUsableFluxes() const;

    std::vector<QDateTime>                             allTimeStepDatesFromEclipseReader() const;
    std::vector<QDateTime>                             timeStepDates() const;
    std::vector<QDateTime>                             timeStepDates(const RigEclipseResultAddress& resVarAddr) const;
    std::vector<double>                                daysSinceSimulationStart() const;
    std::vector<double>                                daysSinceSimulationStart(const RigEclipseResultAddress& resVarAddr) const;
    int                                                reportStepNumber(const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex) const;
    
    std::vector<RigEclipseTimeStepInfo>                timeStepInfos(const RigEclipseResultAddress& resVarAddr) const;
    void                                               setTimeStepInfos(const RigEclipseResultAddress& resVarAddr, const std::vector<RigEclipseTimeStepInfo>& timeStepInfos);

    size_t                                             findOrLoadScalarResultForTimeStep(RiaDefines::ResultCatType type, const QString& resultName, size_t timeStepIndex);
    size_t                                             findOrLoadScalarResult(RiaDefines::ResultCatType type, const QString& resultName);
    size_t                                             findOrLoadScalarResult(const QString& resultName); ///< Simplified search. Assumes unique names across types.

    // Find or create a slot for the results

    size_t                                             findOrCreateScalarResultIndex(RiaDefines::ResultCatType type, const QString& resultName, bool needsToBeStored);
    size_t                                             findScalarResultIndex(RiaDefines::ResultCatType type, const QString& resultName) const;
    size_t                                             findScalarResultIndex(const QString& resultName) const;

    QString                                            makeResultNameUnique(const QString& resultNameProposal) const;

    void                                               createPlaceholderResultEntries();
    void                                               computeDepthRelatedResults();
    void                                               computeCellVolumes();

    void                                               clearScalarResult(RiaDefines::ResultCatType type, const QString & resultName);
    void                                               clearScalarResult(const RigEclipseResultInfo& resultInfo);
    void                                               clearAllResults();
    void                                               freeAllocatedResultsData();
    bool                                               isResultLoaded(const RigEclipseResultInfo& resultInfo) const;


    // Access the results data

    const std::vector< std::vector<double> > &         cellScalarResults(const RigEclipseResultAddress& resVarAddr) const;
    std::vector< std::vector<double> > &               cellScalarResults(const RigEclipseResultAddress& resVarAddr);
    const std::vector<double>&                         cellScalarResults(const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex) const;
    std::vector<double>&                               cellScalarResults(const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex);

    bool                                               updateResultName(RiaDefines::ResultCatType resultType, QString& oldName, const QString& newName);

    static const std::vector<double>*                  getResultIndexableStaticResult(RigActiveCellInfo* actCellInfo,
                                                                                      RigCaseCellResultsData* gridCellResults,
                                                                                      QString porvResultName,
                                                                                      std::vector<double> &activeCellsResultsTempContainer);

public:
    const std::vector<RigEclipseResultInfo>&           infoForEachResultIndex();

    bool                                               mustBeCalculated(size_t scalarResultIndex) const;
    void                                               setMustBeCalculated(size_t scalarResultIndex);
    void                                               eraseAllSourSimData();

    
public:
    size_t                                             addStaticScalarResult(RiaDefines::ResultCatType type, 
                                                                             const QString& resultName, 
                                                                             bool needsToBeStored,
                                                                             size_t resultValueCount);

    bool
                                                       findTransmissibilityResults(size_t& tranX, size_t& tranY, size_t& tranZ) const;
private: // from RimReservoirCellResultsStorage
    void                                               computeSOILForTimeStep(size_t timeStepIndex);
    void                                               testAndComputeSgasForTimeStep(size_t timeStepIndex);

    void                                               computeRiTransComponent(const QString& riTransComponentResultName);
    void                                               computeNncCombRiTrans();

    void                                               computeRiMULTComponent(const QString& riMultCompName);
    void                                               computeNncCombRiMULT();
    void                                               computeRiTRANSbyAreaComponent(const QString& riTransByAreaCompResultName);
    void                                               computeNncCombRiTRANSbyArea();

    void                                               computeCompletionTypeForTimeStep(size_t timeStep);
    double                                             darchysValue();

    void                                               computeOilVolumes();
    void                                               computeMobilePV();

    bool                                               isDataPresent(size_t scalarResultIndex) const;

    void                                               assignValuesToTemporaryLgrs(const QString& resultName, std::vector<double>& values);

    cvf::ref<RifReaderInterface>                       m_readerInterface;

private:
    std::vector< std::vector< std::vector<double> > >  m_cellScalarResults; ///< Scalar results on the complete reservoir for each Result index (ResultVariable) and timestep 
    cvf::Collection<RigStatisticsDataCache>            m_statisticsDataCache;
    RigStatisticsDataCache*                            statistics(const RigEclipseResultAddress& resVarAddr);
private:
    std::vector<RigEclipseResultInfo>                  m_resultInfos;

    RigMainGrid*                                       m_ownerMainGrid;
    RigEclipseCaseData*                                m_ownerCaseData;
    RigActiveCellInfo*                                 m_activeCellInfo;    
};
