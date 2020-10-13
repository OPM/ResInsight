/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"
#include "cafSignal.h"

#include "RimObjectiveFunction.h"

#include <QString>

class RimCustomObjectiveFunctionWeight;
class RimEnsembleCurveSet;
class RimSummaryCase;

//==================================================================================================
///
//==================================================================================================
class RimCustomObjectiveFunction : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    caf::Signal<> objectiveFunctionChanged;

public:
    RimCustomObjectiveFunction();

    RimCustomObjectiveFunctionWeight*              addWeight();
    void                                           removeWeight( RimCustomObjectiveFunctionWeight* weight );
    std::vector<RimCustomObjectiveFunctionWeight*> weights();
    std::vector<double>                            values() const;
    double                                         value( RimSummaryCase* summaryCase ) const;
    std::pair<double, double>                      minMaxValues();
    bool    weightContainsFunctionType( ObjectiveFunction::FunctionType functionType ) const;
    QString title() const;
    bool    isValid() const;
    void    weightUpdated();

private:
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /* = "" */ ) override;
    RimEnsembleCurveSet* parentCurveSet() const;

    caf::PdmFieldHandle* userDescriptionField() override;

private:
    caf::PdmProxyValueField<QString>                           m_functionTitle;
    caf::PdmChildArrayField<RimCustomObjectiveFunctionWeight*> m_weights;
};
