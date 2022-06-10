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

#include "RiaPorosityModel.h"
#include "RimGridCalculationVariable.h"
#include "RimUserDefinedCalculation.h"

#include "cvfArray.h"

class RimEclipseCase;
class RimGridView;

//==================================================================================================
///
///
//==================================================================================================
class RimGridCalculation : public RimUserDefinedCalculation
{
    CAF_PDM_HEADER_INIT;

public:
    RimGridCalculation();

    bool calculate() override;
    void updateDependentObjects() override;
    void removeDependentObjects() override;

protected:
    void onChildrenUpdated( caf::PdmChildArrayFieldHandle*      childArray,
                            std::vector<caf::PdmObjectHandle*>& updatedObjects ) override;

    RimGridCalculationVariable* createVariable() const override;
    std::pair<bool, QString>    validateVariables();

    RimEclipseCase* findEclipseCaseFromVariables() const;

    std::vector<double> getInputVectorForVariable( RimGridCalculationVariable*   v,
                                                   size_t                        tsId,
                                                   RiaDefines::PorosityModelType porosityModel ) const;

    void filterResults( RimGridView*                                 cellFilterView,
                        const std::vector<std::vector<double>>&      values,
                        RimGridCalculationVariable::DefaultValueType defaultValueType,
                        double                                       defaultValue,
                        std::vector<double>&                         resultValues ) const;

    static void replaceFilteredValuesWithVector( const std::vector<double>& inputValues,
                                                 cvf::ref<cvf::UByteArray>  visibility,
                                                 std::vector<double>&       resultValues );

    static void replaceFilteredValuesWithDefaultValue( double                    defaultValue,
                                                       cvf::ref<cvf::UByteArray> visibility,
                                                       std::vector<double>&      resultValues );

    int findFilterVariableIndex() const;

    std::pair<RimGridView*, RimGridCalculationVariable::DefaultValueConfig> findFilterValuesFromVariables() const;
};
