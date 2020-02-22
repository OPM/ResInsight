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

#pragma once

#include "RiaDefines.h"

#include "cafAppEnum.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "cvfObject.h"

#include <QDateTime>

class RimReservoirCellResultsStorageEntryInfo;
class RigCaseCellResultsData;
class RifReaderInterface;
class RigMainGrid;

class RimReservoirCellResultsStorage : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimReservoirCellResultsStorage();
    ~RimReservoirCellResultsStorage() override;

    void setCellResults( RigCaseCellResultsData* cellResults );

    size_t storedResultsCount();

protected:
    // Overridden methods from PdmObject
    void setupBeforeSave() override;

private:
    QString getValidCacheFileName();
    QString getCacheDirectoryPath();
    // Fields
    caf::PdmField<QString>                                            m_resultCacheFileName;
    caf::PdmChildArrayField<RimReservoirCellResultsStorageEntryInfo*> m_resultCacheMetaData;

    RigCaseCellResultsData* m_cellResults;
};

class RimReservoirCellResultsStorageEntryInfo : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimReservoirCellResultsStorageEntryInfo();
    ~RimReservoirCellResultsStorageEntryInfo() override;

    caf::PdmField<caf::AppEnum<RiaDefines::ResultCatType>> m_resultType;
    caf::PdmField<QString>                                 m_resultName;
    caf::PdmField<std::vector<QDateTime>>                  m_timeStepDates;
    caf::PdmField<std::vector<double>>                     m_daysSinceSimulationStart;
    caf::PdmField<qint64>                                  m_filePosition;
};
