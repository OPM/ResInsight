/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017- Statoil ASA
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

#include "cafPdmObject.h"
#include "cafPdmChildArrayField.h"

class RimSummaryCase;
class RimObservedData;
class QFile;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class RimObservedDataCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimObservedDataCollection();
    ~RimObservedDataCollection() override;

    void                            removeObservedData(RimObservedData* observedData);
    RimObservedData*                createAndAddRsmObservedDataFromFile(const QString& fileName, QString* errorText = nullptr);
    RimObservedData*                createAndAddCvsObservedDataFromFile(const QString& fileName, bool useSavedFieldsValuesInDialog, QString* errorText = nullptr);
    std::vector<RimSummaryCase*>    allObservedData();

private:
    bool                            fileExists(const QString& fileName, QString* errorText = nullptr);

private:
    caf::PdmChildArrayField<RimObservedData*> m_observedDataArray;
};
