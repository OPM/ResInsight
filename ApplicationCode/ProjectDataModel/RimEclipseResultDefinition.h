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

#include "RimDefines.h"

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafPdmPtrField.h"
#include "RigFlowDiagResultAddress.h"

class RigCaseCellResultsData;
class RimEclipseCase;
class RimEclipseView;
class RimReservoirCellResultsStorage;
class RimFlowDiagSolution;

//==================================================================================================
///  
///  
//==================================================================================================
class RimEclipseResultDefinition : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimEclipseResultDefinition();
    virtual ~RimEclipseResultDefinition();

    void                            simpleCopy(const RimEclipseResultDefinition* other);

    void                            setEclipseCase(RimEclipseCase* eclipseCase);

    RimDefines::ResultCatType       resultType() const { return m_resultType(); }
    void                            setResultType(RimDefines::ResultCatType val);
    RimDefines::PorosityModelType   porosityModel() const { return m_porosityModel(); }
    void                            setPorosityModel(RimDefines::PorosityModelType val);
    QString                         resultVariable() const { return m_resultVariable(); }
    virtual void                    setResultVariable(const QString& val);
    RimFlowDiagSolution*            flowDiagSolution();
    RigFlowDiagResultAddress        flowDiagResAddress() const;

    QString                         resultVariableUiName();

    void                            loadResult();
    size_t                          scalarResultIndex() const;
    bool                            hasStaticResult() const;
    bool                            hasDynamicResult() const;
    bool                            hasResult() const;
    bool                            isTernarySaturationSelected() const;
    bool                            hasCategoryResult() const;

    RimReservoirCellResultsStorage* currentGridCellResults() const;

    void                            updateResultNameHasChanged();
    void                            updateAnyFieldHasChanged();

protected:
    virtual void                    updateLegendCategorySettings() {};

    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly);
    virtual void                          fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual void                          initAfterRead();
    virtual void                          defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

protected:
    caf::PdmField< caf::AppEnum< RimDefines::ResultCatType > >      m_resultType;
    caf::PdmField< caf::AppEnum< RimDefines::PorosityModelType > >  m_porosityModel;
    caf::PdmField<QString>                                          m_resultVariable;

    caf::PdmPtrField<RimFlowDiagSolution*>                          m_flowSolution;
    caf::PdmField<std::vector<QString> >                            m_selectedTracers;

    friend class RimEclipsePropertyFilter;
    friend class RimEclipseFaultColors;
    friend class RimWellLogExtractionCurve;

    // User interface only fields, to support "filtering"-like behaviour etc.

    caf::PdmField< caf::AppEnum< RimDefines::ResultCatType > >      m_resultTypeUiField;
    caf::PdmField< caf::AppEnum< RimDefines::PorosityModelType > >  m_porosityModelUiField;
    caf::PdmField<QString>                                          m_resultVariableUiField;

    caf::PdmPtrField<RimFlowDiagSolution*>                          m_flowSolutionUiField;
    caf::PdmField<std::vector<QString> >                            m_selectedTracersUiField;


    caf::PdmPointer<RimEclipseCase>                                 m_eclipseCase;

private:
    void                            setFlowSolution(RimFlowDiagSolution* flowSol);
    void                            setSelectedTracers(const std::vector<QString>& selectedTracers);

    bool                            hasDualPorFractureResult();

    QList<caf::PdmOptionItemInfo>   calcOptionsForVariableUiFieldStandard();
    QStringList                     getResultNamesForCurrentUiResultType();
    static void                     removePerCellFaceOptionItems(QList<caf::PdmOptionItemInfo>& optionItems);
};

