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


class RimEclipseView;
class RigCaseCellResultsData;
class RimReservoirCellResultsStorage;
class RimEclipseCase;
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

    void                            setEclipseCase(RimEclipseCase* eclipseCase);

    RimDefines::ResultCatType       resultType() const { return m_resultType(); }
    void                            setResultType(RimDefines::ResultCatType val);
    RimDefines::PorosityModelType   porosityModel() const { return m_porosityModel(); }
    void                            setPorosityModel(RimDefines::PorosityModelType val);
    QString                         resultVariable() const { return m_resultVariable(); }
    virtual void                    setResultVariable(const QString& val);

    void                            loadResult();
    size_t                          scalarResultIndex() const;
    bool                            hasStaticResult() const;
    bool                            hasDynamicResult() const;
    bool                            hasResult() const;
    bool                            isTernarySaturationSelected() const;
    bool                            hasCategoryResult() const;

    RimReservoirCellResultsStorage* currentGridCellResults() const;

    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly);
    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);



    virtual void initAfterRead();
    
    virtual void updateLegendCategorySettings() {};

    void            updateResultNameHasChanged();
    void            updateAnyFieldHasChanged();

protected:
    void            updateFieldVisibility();

protected:
    caf::PdmField< caf::AppEnum< RimDefines::ResultCatType > >      m_resultType;
    caf::PdmField< caf::AppEnum< RimDefines::PorosityModelType > >  m_porosityModel;
    caf::PdmField<QString>                                          m_resultVariable;

    friend class RimEclipsePropertyFilter;
    friend class RimEclipseFaultColors;
    friend class RimWellLogExtractionCurve;

    // User interface only fields, to support "filtering"-like behaviour etc.
    caf::PdmField< caf::AppEnum< RimDefines::ResultCatType > >      m_resultTypeUiField;
    caf::PdmField< caf::AppEnum< RimDefines::PorosityModelType > >  m_porosityModelUiField;
    caf::PdmField<QString>                                          m_resultVariableUiField;

    caf::PdmPointer<RimEclipseCase>                                 m_eclipseCase;

private:
    QList<caf::PdmOptionItemInfo>   calculateValueOptionsForSpecifiedDerivedListPosition(bool showDerivedResultsFirstInList, const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly);
    QStringList                     getResultVariableListForCurrentUIFieldSettings();
    static void                     removePerCellFaceOptionItems(QList<caf::PdmOptionItemInfo>& optionItems);
};

