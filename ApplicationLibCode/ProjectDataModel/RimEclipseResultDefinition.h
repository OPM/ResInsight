/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RiaDefines.h"
#include "RiaPorosityModel.h"

#include "RigFlowDiagResultAddress.h"
#include "RimFlowDiagSolution.h"

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafPdmPtrField.h"
#include "cafPdmUiItem.h"

#include <limits>

#include "RigCaseCellResultsData.h"

class RigCaseCellResultsData;
class RimEclipseCase;
class RimEclipseView;
class RimReservoirCellResultsStorage;
class RimRegularLegendConfig;
class RimTernaryLegendConfig;

//==================================================================================================
///
///
//==================================================================================================
class RimEclipseResultDefinition : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum FlowTracerSelectionType
    {
        FLOW_TR_INJ_AND_PROD,
        FLOW_TR_PRODUCERS,
        FLOW_TR_INJECTORS,
        FLOW_TR_BY_SELECTION
    };
    typedef caf::AppEnum<RimEclipseResultDefinition::FlowTracerSelectionType> FlowTracerSelectionEnum;

    enum FlowTracerSelectionState
    {
        NONE_SELECTED,
        ONE_SELECTED,
        MULTIPLE_SELECTED,
        ALL_SELECTED
    };

public:
    RimEclipseResultDefinition( caf::PdmUiItemInfo::LabelPosType labelPosition = caf::PdmUiItemInfo::LEFT );
    ~RimEclipseResultDefinition() override;

    void simpleCopy( const RimEclipseResultDefinition* other );

    void            setEclipseCase( RimEclipseCase* eclipseCase );
    RimEclipseCase* eclipseCase() const;

    RiaDefines::ResultCatType     resultType() const { return m_resultType(); }
    void                          setResultType( RiaDefines::ResultCatType val );
    RiaDefines::PorosityModelType porosityModel() const { return m_porosityModel(); }
    void                          setPorosityModel( RiaDefines::PorosityModelType val );
    QString                       resultVariable() const { return m_resultVariable(); }
    virtual void                  setResultVariable( const QString& val );
    RiaDefines::PhaseType         resultPhaseType() const;

    void                     setFlowSolution( RimFlowDiagSolution* flowSol );
    RimFlowDiagSolution*     flowDiagSolution() const;
    RigFlowDiagResultAddress flowDiagResAddress() const;

    void setFlowDiagTracerSelectionType( FlowTracerSelectionType selectionType );

    QString resultVariableUiName() const;
    QString resultVariableUiShortName() const;

    void    enableDeltaResults( bool enable );
    int     timeLapseBaseTimeStep() const;
    int     caseDiffIndex() const;
    QString additionalResultText() const;

    void                    loadResult();
    RigEclipseResultAddress eclipseResultAddress() const;
    void                    setFromEclipseResultAddress( const RigEclipseResultAddress& resultAddress );
    bool                    hasStaticResult() const;
    bool                    hasDynamicResult() const;
    bool                    hasResult() const;
    bool                    isTernarySaturationSelected() const;
    bool                    isCompletionTypeSelected() const;
    bool                    hasCategoryResult() const;
    bool                    isFlowDiagOrInjectionFlooding() const;

    RigCaseCellResultsData* currentGridCellResults() const;

    void loadDataAndUpdate();
    void updateAnyFieldHasChanged();

    void setTofAndSelectTracer( const QString& tracerName );
    void setSelectedTracers( const std::vector<QString>& selectedTracers );
    void setSelectedInjectorTracers( const std::vector<QString>& selectedTracers );
    void setSelectedProducerTracers( const std::vector<QString>& selectedTracers );
    void setSelectedSouringTracers( const std::vector<QString>& selectedTracers );

    void updateUiFieldsFromActiveResult();

    bool hasDualPorFractureResult();

    static QList<caf::PdmOptionItemInfo> calcOptionsForVariableUiFieldStandard( RiaDefines::ResultCatType resultCatType,
                                                                                const RigCaseCellResultsData* results,
                                                                                bool showDerivedResultsFirst   = false,
                                                                                bool addPerCellFaceOptionItems = false,
                                                                                bool enableTernary = false );

    void setTernaryEnabled( bool enabled );

    void updateRangesForExplicitLegends( RimRegularLegendConfig* legendConfig,
                                         RimTernaryLegendConfig* ternaryLegendConfig,
                                         int                     currentTimeStep );
    void updateLegendTitle( RimRegularLegendConfig* legendConfig, const QString& legendHeading );

protected:
    virtual void updateLegendCategorySettings(){};

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;
    void                          fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    void initAfterRead() override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;

protected:
    caf::PdmField<caf::AppEnum<RiaDefines::ResultCatType>>     m_resultType;
    caf::PdmField<caf::AppEnum<RiaDefines::PorosityModelType>> m_porosityModel;
    caf::PdmField<QString>                                     m_resultVariable;
    caf::PdmField<QString>                                     m_inputPropertyFileName;

    caf::PdmPtrField<RimFlowDiagSolution*> m_flowSolution;
    caf::PdmField<std::vector<QString>>    m_selectedInjectorTracers;
    caf::PdmField<std::vector<QString>>    m_selectedProducerTracers;
    caf::PdmField<std::vector<QString>>    m_selectedSouringTracers;

    friend class RimEclipsePropertyFilter;
    friend class RimEclipseFaultColors;
    friend class RimWellLogExtractionCurve;

    // User interface only fields, to support "filtering"-like behaviour etc.

    caf::PdmField<caf::AppEnum<RiaDefines::ResultCatType>> m_resultTypeUiField;

    // TODO: Remove Ui field, as behavior is similar to time difference fields
    caf::PdmField<caf::AppEnum<RiaDefines::PorosityModelType>> m_porosityModelUiField;

    caf::PdmField<QString> m_resultVariableUiField;

    caf::PdmField<caf::AppEnum<FlowTracerSelectionType>>        m_flowTracerSelectionMode;
    caf::PdmPtrField<RimFlowDiagSolution*>                      m_flowSolutionUiField;
    caf::PdmField<RigFlowDiagResultAddress::PhaseSelectionEnum> m_phaseSelection;
    caf::PdmField<bool>                                         m_showOnlyVisibleCategoriesInLegend;

    caf::PdmField<bool>                 m_syncInjectorToProducerSelection;
    caf::PdmField<bool>                 m_syncProducerToInjectorSelection;
    caf::PdmField<std::vector<QString>> m_selectedInjectorTracersUiField;
    caf::PdmField<std::vector<QString>> m_selectedProducerTracersUiField;

    caf::PdmField<std::vector<QString>> m_selectedSouringTracersUiField;

    caf::PdmPointer<RimEclipseCase> m_eclipseCase;

private:
    struct TracerComp
    {
        bool operator()( const QString& lhs, const QString& rhs ) const;
    };

    caf::PdmField<int>                m_timeLapseBaseTimestep;
    caf::PdmPtrField<RimEclipseCase*> m_differenceCase;
    caf::PdmField<bool>               m_divideByCellFaceArea;

private:
    void assignFlowSolutionFromCase();

    QString flowDiagResUiText( bool shortLabel, int maxTracerStringLength = std::numeric_limits<int>::max() ) const;

    QList<caf::PdmOptionItemInfo> calcOptionsForSelectedTracerField( bool injector );

    QString timeOfFlightString( bool shorter ) const;
    QString maxFractionTracerString( bool shorter ) const;

    QString selectedTracersString() const;

    void               changedTracerSelectionField( bool injector );
    static QStringList getResultNamesForResultType( RiaDefines::ResultCatType     resultCatType,
                                                    const RigCaseCellResultsData* results );

    std::vector<QString>          allTracerNames() const;
    std::set<QString, TracerComp> setOfTracersOfType( bool injector ) const;

    FlowTracerSelectionState injectorSelectionState() const;
    FlowTracerSelectionState producerSelectionState() const;

    void syncInjectorToProducerSelection();
    void syncProducerToInjectorSelection();

    // Delta Case / Delta Time Step / Divide by Cell Face Area
    bool isDeltaResultEnabled() const;
    bool isDeltaCasePossible() const;
    bool isDeltaCaseActive() const;
    bool isDeltaTimeStepPossible() const;
    bool isDeltaTimeStepActive() const;
    bool isDivideByCellFaceAreaPossible() const;
    bool isDivideByCellFaceAreaActive() const;

    QString additionalResultTextShort() const;

    bool showDerivedResultsFirstInVariableUiField() const;
    bool addPerCellFaceOptionsForVariableUiField() const;

    QString getInputPropertyFileName( const QString& resultName ) const;

private:
    bool                             m_isDeltaResultEnabled;
    caf::PdmUiItemInfo::LabelPosType m_labelPosition;
    bool                             m_ternaryEnabled;
};
