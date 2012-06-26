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

class RifReaderInterface;
class RigMainGrid;

//==================================================================================================
/// Class containing the results for the complete number of active cells. Both main grid and LGR's
//==================================================================================================
class RigReservoirCellResults : public cvf::Object
{
public:
    RigReservoirCellResults(RigMainGrid* ownerGrid);

    void                setReaderInterface(RifReaderInterface* readerInterface);

    // Max and min values of the results
    void                recalculateMinMax(size_t scalarResultIndex);
    void                minMaxCellScalarValues(size_t scalarResultIndex, double& min, double& max);
    void                minMaxCellScalarValues(size_t scalarResultIndex, size_t timeStepIndex, double& min, double& max);

    // Access meta-information about the results
    size_t              resultCount() const;
    size_t              timeStepCount(size_t scalarResultIndex) const; 
    size_t              maxTimeStepCount() const; 
    QStringList         resultNames(RimDefines::ResultCatType type) const;
    bool                isUsingGlobalActiveIndex(size_t scalarResultIndex) const;
    QList<QDateTime>    timeStepDates(size_t scalarResultIndex) const;
    void                setTimeStepDates(size_t scalarResultIndex, const QList<QDateTime>& dates);

    // Find or create a slot for the results
    size_t              findOrLoadScalarResult(RimDefines::ResultCatType type, const QString& resultName);
    size_t              findOrLoadScalarResult(const QString& resultName); ///< Simplified search. Assumes unique names across types.
    size_t              findScalarResultIndex(RimDefines::ResultCatType type, const QString& resultName) const;
    size_t              findScalarResultIndex(const QString& resultName) const;
    size_t              addEmptyScalarResult(RimDefines::ResultCatType type, const QString& resultName);
    QString             makeResultNameUnique(const QString& resultNameProposal) const;

    void                removeResult(const QString& resultName);

    void                loadOrComputeSOIL();

    // Access the results data
    std::vector< std::vector<double> > &                    cellScalarResults(size_t scalarResultIndex);
    const std::vector< std::vector<double> >&               cellScalarResults(size_t scalarResultIndex) const;

private:
    std::vector< std::vector< std::vector<double> > >       m_cellScalarResults; ///< Scalar results for each timestep for each Result index (ResultVariable)
    std::vector< std::pair<double, double> >                m_maxMinValues; ///< Max min values for each Result index
    std::vector< std::vector< std::pair<double, double> > > m_maxMinValuesPrTs; ///< Max min values for each timestep and Result index

    class ResultInfo
    {
    public:
        ResultInfo(RimDefines::ResultCatType resultType, QString resultName, size_t gridScalarResultIndex)
            : m_resultType(resultType), m_resultName(resultName), m_gridScalarResultIndex(gridScalarResultIndex) { }

    public:
        RimDefines::ResultCatType   m_resultType;
        QString                     m_resultName;
        size_t                      m_gridScalarResultIndex;
        QList<QDateTime>            m_timeStepDates;
    };

    std::vector<ResultInfo>                                 m_resultInfos;
    cvf::ref<RifReaderInterface>                            m_readerInterface;
    RigMainGrid*                                            m_ownerMainGrid;

};

