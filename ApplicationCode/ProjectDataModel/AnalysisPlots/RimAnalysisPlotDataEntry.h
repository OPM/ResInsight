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

#include "RiaSummaryCurveDefinition.h"
#include "RifEclipseSummaryAddress.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

class RimSummaryCase;
class RimSummaryAddress;
class RimSummaryCaseCollection;

class RimAnalysisPlotDataEntry : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimAnalysisPlotDataEntry();
    ~RimAnalysisPlotDataEntry() override;

    void                      setFromCurveDefinition( const RiaSummaryCurveDefinition& curveDef );
    RiaSummaryCurveDefinition curveDefinition() const;

    RimSummaryCase*           summaryCase() const;
    RimSummaryCaseCollection* ensemble() const;
    RifEclipseSummaryAddress  summaryAddress() const;
    bool                      isEnsembleCurve() const;

private:
    caf::PdmPtrField<RimSummaryCase*>           m_summaryCase;
    caf::PdmPtrField<RimSummaryCaseCollection*> m_ensemble;
    caf::PdmChildField<RimSummaryAddress*>      m_summaryAddress;
    caf::PdmField<bool>                         m_isEnsembleCurve;
};
