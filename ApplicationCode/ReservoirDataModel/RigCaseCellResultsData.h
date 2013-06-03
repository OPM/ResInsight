/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RimDefines.h"
#include <QDateTime>
#include <vector>
#include <cmath>
#include "RifReaderInterface.h"

class RifReaderInterface;
class RigMainGrid;

//==================================================================================================
/// Class containing the results for the complete number of active cells. Both main grid and LGR's
//==================================================================================================
class RigCaseCellResultsData : public cvf::Object
{
public:
    RigCaseCellResultsData(RigMainGrid* ownerGrid);

    void                                               setMainGrid(RigMainGrid* ownerGrid);

    // Max and min values of the results
    void                                               recalculateMinMax(size_t scalarResultIndex);
    void                                               minMaxCellScalarValues(size_t scalarResultIndex, double& min, double& max);
    void                                               minMaxCellScalarValues(size_t scalarResultIndex, size_t timeStepIndex, double& min, double& max);
    void                                               posNegClosestToZero(size_t scalarResultIndex, double& pos, double& neg);
    void                                               posNegClosestToZero(size_t scalarResultIndex, size_t timeStepIndex, double& pos, double& neg);
    const std::vector<size_t>&                         cellScalarValuesHistogram(size_t scalarResultIndex);
    void                                               p10p90CellScalarValues(size_t scalarResultIndex, double& p10, double& p90);
    void                                               meanCellScalarValues(size_t scalarResultIndex, double& meanValue);

    // Access meta-information about the results
    size_t                                             resultCount() const;
    size_t                                             timeStepCount(size_t scalarResultIndex) const; 
    size_t                                             maxTimeStepCount(size_t* scalarResultIndex = NULL) const; 
    QStringList                                        resultNames(RimDefines::ResultCatType type) const;
    bool                                               isUsingGlobalActiveIndex(size_t scalarResultIndex) const;

    QDateTime                                          timeStepDate(size_t scalarResultIndex, size_t timeStepIndex) const;
    std::vector<QDateTime>                             timeStepDates(size_t scalarResultIndex) const;
    void                                               setTimeStepDates(size_t scalarResultIndex, const std::vector<QDateTime>& dates);

    // Find or create a slot for the results

    size_t                                             findScalarResultIndex(RimDefines::ResultCatType type, const QString& resultName) const;
    size_t                                             findScalarResultIndex(const QString& resultName) const;

    size_t                                             addEmptyScalarResult(RimDefines::ResultCatType type, const QString& resultName, bool needsToBeStored);
    QString                                            makeResultNameUnique(const QString& resultNameProposal) const;

    void                                               removeResult(const QString& resultName);
    void                                               clearAllResults();

    // Access the results data

    const std::vector< std::vector<double> > &         cellScalarResults(size_t scalarResultIndex) const;
    std::vector< std::vector<double> > &               cellScalarResults(size_t scalarResultIndex);
    std::vector<double>&                               cellScalarResults(size_t scalarResultIndex, size_t timeStepIndex);
    double                                             cellScalarResult(size_t scalarResultIndex, size_t timeStepIndex, size_t resultValueIndex);

    static RifReaderInterface::PorosityModelResultType convertFromProjectModelPorosityModel(RimDefines::PorosityModelType porosityModel);

public:
    class ResultInfo
    {
    public:
        ResultInfo(RimDefines::ResultCatType resultType, bool needsToBeStored, bool mustBeCalculated, QString resultName, size_t gridScalarResultIndex)
            : m_resultType(resultType), m_needsToBeStored(needsToBeStored), m_resultName(resultName), m_gridScalarResultIndex(gridScalarResultIndex), m_mustBeCalculated(mustBeCalculated) { }

    public:
        RimDefines::ResultCatType   m_resultType;
        bool                        m_needsToBeStored;
        bool                        m_mustBeCalculated;
        QString                     m_resultName;
        size_t                      m_gridScalarResultIndex;
        std::vector<QDateTime>      m_timeStepDates;
    };

    const std::vector<ResultInfo>&                          infoForEachResultIndex() { return m_resultInfos;}

    bool                                                    mustBeCalculated(size_t scalarResultIndex) const;
    void                                                    setMustBeCalculated(size_t scalarResultIndex);

    
public:
    size_t                                                  addStaticScalarResult(RimDefines::ResultCatType type, 
                                                                                  const QString& resultName, 
                                                                                  bool needsToBeStored,
                                                                                  size_t resultValueCount);

private:
    std::vector< std::vector< std::vector<double> > >       m_cellScalarResults; ///< Scalar results on the complete reservoir for each Result index (ResultVariable) and timestep 
    std::vector< std::pair<double, double> >                m_maxMinValues;      ///< Max min values for each Result index
    std::vector< std::pair<double, double> >                m_posNegClosestToZero;
    std::vector< std::vector<size_t> >                      m_histograms;        ///< Histogram for each Result Index
    std::vector< std::pair<double, double> >                m_p10p90;            ///< P10 and p90 values for each Result Index
    std::vector< double >                                   m_meanValues;        ///< Mean value for each Result Index

    std::vector< std::vector< std::pair<double, double> > > m_maxMinValuesPrTs;  ///< Max min values for each Result index and timestep
    std::vector< std::vector< std::pair<double, double> > > m_posNegClosestToZeroPrTs;



private:
    std::vector<ResultInfo>                                 m_resultInfos;

    RigMainGrid*                                            m_ownerMainGrid;

};
