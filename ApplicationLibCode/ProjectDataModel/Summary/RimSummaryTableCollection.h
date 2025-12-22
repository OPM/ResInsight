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

#include "RifEclipseSummaryAddressDefines.h"

#include "RimAbstractPlotCollection.h"
#include "RimSummaryTable.h"

#include "cafPdmObjectCollection.h"

class RimSummaryCase;

//==================================================================================================
///
/// Collection for managing summary tables
///
//==================================================================================================
class RimSummaryTableCollection : public caf::PdmObjectCollection<RimSummaryTable>, public RimPlotCollection
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryTableCollection();
    ~RimSummaryTableCollection() override;

    // RimPlotCollection interface
    void   deleteAllPlots() override;
    void   loadDataAndUpdateAllPlots() override;
    size_t plotCount() const override;

    // Convenience accessors (delegate to base class)
    std::vector<RimSummaryTable*> tables() const;
    void                          addTable( RimSummaryTable* table );
    void                          removeTable( RimSummaryTable* table );

    // Domain-specific creation methods
    RimSummaryTable* createDefaultSummaryTable();
    RimSummaryTable* createSummaryTableFromCategoryAndVectorName( RimSummaryCase*                                  summaryCase,
                                                                  RifEclipseSummaryAddressDefines::SummaryCategory category,
                                                                  const QString&                                   vectorName );
};
