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

#include "cafPdmChildArrayField.h"
#include "cafPdmObject.h"

class RimObservedFmuRftData;
class RimObservedSummaryData;
class RimPressureDepthData;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RimObservedDataCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimObservedDataCollection();

    void                    removeObservedSummaryData( RimObservedSummaryData* observedSummaryData );
    void                    removeObservedFmuRftData( RimObservedFmuRftData* observedFmuRftData );
    RimObservedSummaryData* createAndAddRsmObservedSummaryDataFromFile( const QString& fileName, QString* errorText = nullptr );
    RimObservedSummaryData*
        createAndAddCvsObservedSummaryDataFromFile( const QString& fileName, bool useSavedFieldsValuesInDialog, QString* errorText = nullptr );
    RimObservedFmuRftData*               createAndAddFmuRftDataFromPath( const QString& directoryPath );
    RimPressureDepthData*                createAndAddPressureDepthDataFromPath( const QString& fileName );
    std::vector<RimObservedSummaryData*> allObservedSummaryData() const;
    std::vector<RimObservedFmuRftData*>  allObservedFmuRftData() const;
    std::vector<RimPressureDepthData*>   allPressureDepthData() const;

private:
    bool fileExists( const QString& fileName, QString* errorText = nullptr );
    void onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects ) override;

private:
    caf::PdmChildArrayField<RimObservedSummaryData*> m_observedDataArray;
    caf::PdmChildArrayField<RimObservedFmuRftData*>  m_observedFmuRftArray;
    caf::PdmChildArrayField<RimPressureDepthData*>   m_observedPressureDepthArray;
};
