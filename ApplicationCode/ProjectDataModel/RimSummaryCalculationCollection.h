/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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
#include "cafPdmChildField.h"
#include "cafPdmObject.h"

class RimSummaryCalculation;
class RimSummaryCase;
class RimCalculatedSummaryCase;

//==================================================================================================
///
///
//==================================================================================================
class RimSummaryCalculationCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryCalculationCollection();

    RimSummaryCalculation*              addCalculation();
    RimSummaryCalculation*              addCalculationCopy( const RimSummaryCalculation* sourceCalculation );
    void                                deleteCalculation( RimSummaryCalculation* calculation );
    std::vector<RimSummaryCalculation*> calculations() const;
    RimSummaryCalculation*              findCalculationById( int id ) const;

    RimSummaryCase* calculationSummaryCase();

    void deleteAllContainedObjects();
    void rebuildCaseMetaData();

private:
    void initAfterRead() override;

private:
    caf::PdmChildArrayField<RimSummaryCalculation*> m_calculations;
    caf::PdmChildField<RimCalculatedSummaryCase*>   m_calcuationSummaryCase;
};
