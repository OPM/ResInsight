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

class RigCaseCellResultsData;
class RimEclipseCase;
class RimEclipseView;
class RimReservoirCellResultsStorage;


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

    enum FlowTracerSelectionNumbers
    {
        NONE_SELECTED,
        ONE_SELECTED,
        MULTIPLE_SELECTED
    };

public:
    RimEclipseResultDefinition();
    virtual ~RimEclipseResultDefinition();

    void                            simpleCopy(const RimEclipseResultDefinition* other);

    void                            setEclipseCase(RimEclipseCase* eclipseCase);

    RiaDefines::ResultCatType       resultType() const { return m_resultType(); }
    void                            setResultType(RiaDefines::ResultCatType val);
    RiaDefines::PorosityModelType   porosityModel() const { return m_porosityModel(); }
    void                            setPorosityModel(RiaDefines::PorosityModelType val);
    QString                         resultVariable() const { return m_resultVariable(); }
    virtual void                    setResultVariable(const QString& val);
    
    void                            setFlowSolution(RimFlowDiagSolution* flowSol);
    RimFlowDiagSolution*            flowDiagSolution();
    RigFlowDiagResultAddress        flowDiagResAddress() const;

    void                            setFlowDiagTracerSelectionType(FlowTracerSelectionType selectionType);

    QString                         resultVariableUiName() const;
    QString                         resultVariableUiShortName() const;

    void                            loadResult();
    size_t                          scalarResultIndex() const;
    bool                            hasStaticResult() const;
    bool                            hasDynamicResult() const;
    bool                            hasResult() const;
    bool                            isTernarySaturationSelected() const;
    bool                            isCompletionTypeSelected() const;
    bool                            hasCategoryResult() const;
    bool                            isFlowDiagOrInjectionFlooding() const;

    RigCaseCellResultsData*         currentGridCellResults() const;

    void                            loadDataAndUpdate();
    void                            updateAnyFieldHasChanged();

    void                            setTofAndSelectTracer(const QString& tracerName);
    void                            setSelectedTracers(const std::vector<QString>& selectedTracers);
    void                            setSelectedInjectorTracers(const std::vector<QString>& selectedTracers);
    void                            setSelectedProducerTracers(const std::vector<QString>& selectedTracers);
    void                            setSelectedSouringTracers(const std::vector<QString>& selectedTracers);

    void                            updateUiFieldsFromActiveResult();

protected:
    virtual void                    updateLegendCategorySettings() {};

    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly);
    virtual void                          fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);

    virtual void                          initAfterRead();
    virtual void                          defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    virtual void                          defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute) override;

protected:
    caf::PdmField< caf::AppEnum< RiaDefines::ResultCatType > >      m_resultType;
    caf::PdmField< caf::AppEnum< RiaDefines::PorosityModelType > >  m_porosityModel;
    caf::PdmField<QString>                                          m_resultVariable;

    caf::PdmPtrField<RimFlowDiagSolution*>                          m_flowSolution;    
    caf::PdmField<std::vector<QString> >                            m_selectedInjectorTracers;
    caf::PdmField<std::vector<QString> >                            m_selectedProducerTracers;
    caf::PdmField<std::vector<QString> >                            m_selectedSouringTracers;

    friend class RimEclipsePropertyFilter;
    friend class RimEclipseFaultColors;
    friend class RimWellLogExtractionCurve;

    // User interface only fields, to support "filtering"-like behaviour etc.

    caf::PdmField< caf::AppEnum< RiaDefines::ResultCatType > >      m_resultTypeUiField;
    caf::PdmField< caf::AppEnum< RiaDefines::PorosityModelType > >  m_porosityModelUiField;
    caf::PdmField<QString>                                          m_resultVariableUiField;

    caf::PdmField< caf::AppEnum< FlowTracerSelectionType > >        m_flowTracerSelectionMode;
    caf::PdmPtrField<RimFlowDiagSolution*>                          m_flowSolutionUiField;
    caf::PdmField< RigFlowDiagResultAddress::PhaseSelectionEnum >   m_phaseSelection;
    
    caf::PdmField<std::vector<QString> >                            m_selectedInjectorTracersUiField;
    caf::PdmField<std::vector<QString> >                            m_selectedProducerTracersUiField;

    caf::PdmField<std::vector<QString> >                            m_selectedSouringTracersUiField;


    caf::PdmPointer<RimEclipseCase>                                 m_eclipseCase;

    caf::PdmField<std::vector<QString> >                            m_selectedTracers_OBSOLETE;

private:
    void                            assignFlowSolutionFromCase();

    bool                            hasDualPorFractureResult();

    QList<caf::PdmOptionItemInfo>   calcOptionsForVariableUiFieldStandard();
    QList<caf::PdmOptionItemInfo>   calcOptionsForSelectedTracerField(bool injector);
    caf::PdmOptionItemInfo          calcOptionForTimeOfFlightField();
    caf::PdmOptionItemInfo          calcOptionForMaxFractionTracerField();

    void                            changedTracerSelectionField(bool injector);
    QStringList                     getResultNamesForCurrentUiResultType();
    static void                     removePerCellFaceOptionItems(QList<caf::PdmOptionItemInfo>& optionItems);

    std::vector<QString>            allTracerNames() const;
    
    FlowTracerSelectionNumbers      injectorTracersSelected() const;
    FlowTracerSelectionNumbers      producerTracersSelected() const;
};

