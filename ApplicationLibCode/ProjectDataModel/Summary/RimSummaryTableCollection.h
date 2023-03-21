/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RimAbstractPlotCollection.h"
#include "RimSummaryTable.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmObject.h"

class RimSummaryCase;

//==================================================================================================
///
///
//==================================================================================================
class RimSummaryTableCollection : public caf::PdmObject, public RimPlotCollection
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryTableCollection();
    ~RimSummaryTableCollection() override;

    void   deleteAllPlots() override;
    void   loadDataAndUpdateAllPlots() override;
    size_t plotCount() const override;

    std::vector<RimSummaryTable*> tables() const;
    size_t                        tableCount() const;
    void                          addTable( RimSummaryTable* table );
    void                          insertTable( RimSummaryTable* table, size_t index );
    void                          removeTable( RimSummaryTable* table );

    RimSummaryTable* createDefaultSummaryTable();
    RimSummaryTable* createSummaryTableFromCategoryAndVectorName( RimSummaryCase*                              summaryCase,
                                                                  RifEclipseSummaryAddress::SummaryVarCategory category,
                                                                  const QString&                               vectorName );

    void updateSummaryNameHasChanged();

private:
    caf::PdmChildArrayField<RimSummaryTable*> m_summaryTables;
};
