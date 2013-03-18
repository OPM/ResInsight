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

#include <QDateTime>
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafAppEnum.h"
#include "RimDefines.h"

class RimReservoirCellResultsCacheEntryInfo;
class RigReservoirCellResults;
class RifReaderInterface;
class RigMainGrid;

class RimReservoirCellResultsCacher : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimReservoirCellResultsCacher();
    virtual ~RimReservoirCellResultsCacher();

    // Fields
    caf::PdmField<QString>      m_resultCacheFileName;
    caf::PdmPointersField<RimReservoirCellResultsCacheEntryInfo*> 
                                m_resultCacheMetaData;

    RigReservoirCellResults*       cellResults()  { return m_cellResults; }
    const RigReservoirCellResults* cellResults() const  { return m_cellResults; }

    void                        setCellResults(RigReservoirCellResults* cellResults);
    void                        setMainGrid(RigMainGrid* mainGrid);

    void                        setReaderInterface(RifReaderInterface* readerInterface);
    RifReaderInterface*         readerInterface();

    void                        loadOrComputeSOIL();
    void                        loadOrComputeSOILForTimeStep(size_t timeStepIndex);
    void                        computeDepthRelatedResults();

    size_t                      findOrLoadScalarResultForTimeStep(RimDefines::ResultCatType type, const QString& resultName, size_t timeStepIndex);
    size_t                      findOrLoadScalarResult(RimDefines::ResultCatType type, const QString& resultName);
    size_t                      findOrLoadScalarResult(const QString& resultName); ///< Simplified search. Assumes unique names across types.

    // Overridden methods from PdmObject
    virtual void                setupBeforeSave();

private:
    QString                     getValidCacheFileName();
    QString                     getCacheDirectoryPath();
    cvf::ref<RifReaderInterface>                            m_readerInterface;
    RigReservoirCellResults*    m_cellResults;
    RigMainGrid*                m_ownerMainGrid;
};

class RimReservoirCellResultsCacheEntryInfo : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimReservoirCellResultsCacheEntryInfo();
    virtual ~RimReservoirCellResultsCacheEntryInfo();

    caf::PdmField<caf::AppEnum< RimDefines::ResultCatType> > m_resultType;
    caf::PdmField<QString>                                   m_resultName;
    caf::PdmField< std::vector <QDateTime> >                 m_timeStepDates;
    caf::PdmField< std::vector <int> >                       m_timeStepHasData;
    caf::PdmField<qint64>                                    m_filePosition;
};

