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

    void                                 removeObservedSummaryData( RimObservedSummaryData* observedSummaryData );
    void                                 removeObservedFmuRftData( RimObservedFmuRftData* observedFmuRftData );
    RimObservedSummaryData*              createAndAddRsmObservedSummaryDataFromFile( const QString& fileName,
                                                                                     QString*       errorText = nullptr );
    RimObservedSummaryData*              createAndAddCvsObservedSummaryDataFromFile( const QString& fileName,
                                                                                     bool           useSavedFieldsValuesInDialog,
                                                                                     QString*       errorText = nullptr );
    RimObservedFmuRftData*               createAndAddFmuRftDataFromPath( const QString& directoryPath );
    std::vector<RimObservedSummaryData*> allObservedSummaryData() const;
    std::vector<RimObservedFmuRftData*>  allObservedFmuRftData() const;

private:
    bool fileExists( const QString& fileName, QString* errorText = nullptr );

private:
    caf::PdmChildArrayField<RimObservedSummaryData*> m_observedDataArray;
    caf::PdmChildArrayField<RimObservedFmuRftData*>  m_observedFmuRftArray;
};
