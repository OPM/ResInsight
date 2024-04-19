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

#include "cafPdmChildField.h"
#include "cafSignal.h"

#include "cvfArray.h"

#include <optional>

class RimEclipseCase;
class RimGridView;
class RigEclipseResultAddress;
class RimEclipseResultAddress;
class RimIdenticalGridCaseGroup;
class RigActiveCellInfo;

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

    enum class AdditionalCasesType
    {
        NONE,
        GRID_CASE_GROUP,
        ALL_CASES
    };

    RimGridCalculation();

    bool preCalculate() const override;
    bool calculate() override;

    void updateDependentObjects() override;
    void removeDependentObjects() override;

    std::vector<RimEclipseCase*> outputEclipseCases() const;
    RigEclipseResultAddress      outputAddress() const;
    bool                         calculateForCases( const std::vector<RimEclipseCase*>& calculationCases,
                                                    cvf::UByteArray*                    inputValueVisibilityFilter,
                                                    std::optional<std::vector<size_t>>  timeSteps,
                                                    bool                                evaluateDependentCalculations );

    void findAndEvaluateDependentCalculations( const std::vector<RimEclipseCase*>& calculationCases,
                                               cvf::UByteArray*                    inputValueVisibilityFilter,
                                               std::optional<std::vector<size_t>>  timeSteps );

    void assignEclipseCaseForNullPointers( RimEclipseCase* eclipseCase );

    std::vector<RimEclipseCase*> inputCases() const;

    RimGridCalculationVariable* createVariable() override;

protected:
    void onChildrenUpdated( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& updatedObjects ) override;

    std::pair<bool, QString> validateVariables();

    std::vector<double> getActiveCellValuesForVariable( RimGridCalculationVariable*   variable,
                                                        size_t                        tsId,
                                                        RiaDefines::PorosityModelType porosityModel,
                                                        RimEclipseCase*               sourceCase,
                                                        RimEclipseCase*               destinationCase ) const;

    std::vector<double> getActiveCellValues( const QString&                  resultName,
                                             const RiaDefines::ResultCatType resultCategoryType,
                                             size_t                          tsId,
                                             RiaDefines::PorosityModelType   porosityModel,
                                             RimEclipseCase*                 sourceCase,
                                             RimEclipseCase*                 destinationCase ) const;

    void filterResults( RimGridView*                            cellFilterView,
                        const std::vector<std::vector<double>>& values,
                        size_t                                  timeStep,
                        RimGridCalculation::DefaultValueType    defaultValueType,
                        double                                  defaultValue,
                        std::vector<double>&                    resultValues,
                        RiaDefines::PorosityModelType           porosityModel,
                        RimEclipseCase*                         outputEclipseCase ) const;

    static void replaceFilteredValuesWithVector( const std::vector<double>& inputValues,
                                                 cvf::ref<cvf::UByteArray>  visibility,
                                                 std::vector<double>&       resultValues,
                                                 RigActiveCellInfo*         activeCellInfo );

    static void replaceFilteredValuesWithDefaultValue( double                    defaultValue,
                                                       cvf::ref<cvf::UByteArray> visibility,
                                                       std::vector<double>&      resultValues,
                                                       RigActiveCellInfo*        activeCellInfo );

    using DefaultValueConfig = std::pair<RimGridCalculation::DefaultValueType, double>;
    DefaultValueConfig defaultValueConfiguration() const;

    QString nonVisibleResultAddressText() const;

    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void                          initAfterRead() override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

private:
    void onVariableUpdated( const SignalEmitter* emitter );
    bool allSourceCasesAreEqualToDestinationCase() const;

    static std::pair<bool, QStringList> createStatisticsText( const std::vector<std::vector<double>>& values );

private:
    caf::PdmPtrField<RimGridView*>                m_cellFilterView;
    caf::PdmField<caf::AppEnum<DefaultValueType>> m_defaultValueType;
    caf::PdmField<double>                         m_defaultValue;
    caf::PdmPtrField<RimEclipseCase*>             m_destinationCase;

    caf::PdmField<caf::AppEnum<AdditionalCasesType>> m_additionalCasesType;
    caf::PdmPtrField<RimIdenticalGridCaseGroup*>     m_additionalCaseGroup;

    caf::PdmField<std::vector<int>> m_selectedTimeSteps;

    caf::PdmProxyValueField<QString>             m_nonVisibleResultText;
    caf::PdmChildField<RimEclipseResultAddress*> m_nonVisibleResultAddress;
    caf::PdmField<bool>                          m_editNonVisibleResultAddress;

    caf::PdmField<bool> m_applyToAllCases_OBSOLETE;

    bool m_releaseMemoryAfterDataIsExtracted = false;
};
