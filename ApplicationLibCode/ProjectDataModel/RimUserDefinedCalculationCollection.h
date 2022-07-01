/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

class RimUserDefinedCalculation;
class RimUserDefinedCase;
class RimCalculatedSummaryCase;

//==================================================================================================
///
///
//==================================================================================================
class RimUserDefinedCalculationCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimUserDefinedCalculationCollection();

    RimUserDefinedCalculation*              addCalculation();
    RimUserDefinedCalculation*              addCalculationCopy( const RimUserDefinedCalculation* sourceCalculation );
    void                                    deleteCalculation( RimUserDefinedCalculation* calculation );
    std::vector<RimUserDefinedCalculation*> calculations() const;
    RimUserDefinedCalculation*              findCalculationById( int id ) const;

    void deleteAllContainedObjects();

    virtual RimUserDefinedCalculation* createCalculation() const = 0;
    virtual void                       rebuildCaseMetaData()     = 0;

    void ensureValidCalculationIds();
    void assignCalculationIdToCalculation( RimUserDefinedCalculation* calculation ) const;

private:
    caf::PdmChildArrayField<RimUserDefinedCalculation*> m_calculations;
};
