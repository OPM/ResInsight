/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017- Statoil ASA
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

#include "cafPdmField.h"
#include "cafPdmObject.h"

class EnsembleParameter;
class RimEnsembleCurveSet;
class RimSummaryCase;
class RimSummaryPlot;
class RimEnsembleCurveFilterCollection;

//==================================================================================================
///  
//==================================================================================================
class RimEnsembleCurveFilter : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimEnsembleCurveFilter();
    RimEnsembleCurveFilter(const QString& ensembleParameterName);

    bool                    isActive() const;
    double                  minValue() const;
    double                  maxValue() const;
    std::set<QString>       categories() const;
    QString                 ensembleParameterName() const;
    QString                 filterId() const;

    QList<caf::PdmOptionItemInfo>   calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly) override;
    void                            fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    void                            defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    void                            defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute) override;

    std::vector<RimSummaryCase*>            applyFilter(const std::vector<RimSummaryCase*>& allSumCases);

    void                                    loadDataAndUpdate();
    EnsembleParameter                       selectedEnsembleParameter() const;

protected:
    caf::PdmFieldHandle*  objectToggleField() override;

private:
    RimEnsembleCurveSet * parentCurveSet() const;
    RimEnsembleCurveFilterCollection* parentCurveFilterCollection() const;
    void                  setInitialValues(bool forceDefault);

private:
    caf::PdmField<bool>                 m_active;
    caf::PdmField<bool>                 m_deleteButton;
    caf::PdmField<QString>              m_ensembleParameterName;
    caf::PdmField<double>               m_minValue;
    caf::PdmField<double>               m_maxValue;
    caf::PdmField<std::vector<QString>> m_categories;

    double                              m_lowerLimit;
    double                              m_upperLimit;
};

