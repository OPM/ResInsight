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

#include "cafSignal.h"
#include "cvfArray.h"

class RimEclipseCase;
class RimGridView;
class RigEclipseResultAddress;

//==================================================================================================
///
///
//==================================================================================================
class RimGridCalculation : public RimUserDefinedCalculation
{
    CAF_PDM_HEADER_INIT;

public:
    enum class DefaultValueType
    {
        POSITIVE_INFINITY,
        FROM_PROPERTY,
        USER_DEFINED
    };

    RimGridCalculation();

    bool calculate() override;
    void updateDependentObjects() override;
    void removeDependentObjects() override;

    RimEclipseCase*         outputEclipseCase() const;
    RigEclipseResultAddress outputAddress() const;

    std::vector<RimEclipseCase*> inputCases() const;

protected:
    void onChildrenUpdated( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& updatedObjects ) override;

    RimGridCalculationVariable* createVariable() override;
    std::pair<bool, QString>    validateVariables();

    std::vector<double> getInputVectorForVariable( RimGridCalculationVariable*   v,
                                                   size_t                        tsId,
                                                   RiaDefines::PorosityModelType porosityModel,
                                                   RimEclipseCase*               outputEclipseCase ) const;

    void filterResults( RimGridView*                            cellFilterView,
                        const std::vector<std::vector<double>>& values,
                        RimGridCalculation::DefaultValueType    defaultValueType,
                        double                                  defaultValue,
                        std::vector<double>&                    resultValues,
                        RiaDefines::PorosityModelType           porosityModel,
                        RimEclipseCase*                         outputEclipseCase ) const;

    static void replaceFilteredValuesWithVector( const std::vector<double>&    inputValues,
                                                 cvf::ref<cvf::UByteArray>     visibility,
                                                 std::vector<double>&          resultValues,
                                                 RiaDefines::PorosityModelType porosityModel,
                                                 RimEclipseCase*               outputEclipseCase );

    static void replaceFilteredValuesWithDefaultValue( double                        defaultValue,
                                                       cvf::ref<cvf::UByteArray>     visibility,
                                                       std::vector<double>&          resultValues,
                                                       RiaDefines::PorosityModelType porosityModel,
                                                       RimEclipseCase*               outputEclipseCase );

    using DefaultValueConfig = std::pair<RimGridCalculation::DefaultValueType, double>;
    DefaultValueConfig defaultValueConfiguration() const;

    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void                          initAfterRead() override;

private:
    void onVariableUpdated( const SignalEmitter* emitter );

private:
    caf::PdmPtrField<RimGridView*>                m_cellFilterView;
    caf::PdmField<caf::AppEnum<DefaultValueType>> m_defaultValueType;
    caf::PdmField<double>                         m_defaultValue;
    caf::PdmPtrField<RimEclipseCase*>             m_destinationCase;
    caf::PdmField<int>                            m_defaultPropertyVariableIndex;
};
