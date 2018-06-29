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

#include "RimEnsembleCurveSet.h"
#include "RimSummaryCase.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"

class RifEclipseSummaryAddress;
class RimSummaryCaseCollection;
class RimEnsembleStatisticsCase;


//==================================================================================================
///  
//==================================================================================================
class RimEnsembleStatistics : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimEnsembleStatistics();

    bool                        isActive() const;
    bool                        basedOnFilteredCases() const { return m_basedOnFilteredCases; }
    bool                        showP10Curve() const { return m_showP10Curve; }
    bool                        showP50Curve() const { return m_showP50Curve; }
    bool                        showP90Curve() const { return m_showP90Curve; }
    bool                        showMeanCurve() const { return m_showMeanCurve; }
    bool                        showCurveLabels() const { return m_showCurveLabels; }
    cvf::Color3f                color() const { return m_color; }

    void                        disableP10Curve(bool disable);
    void                        disableP50Curve(bool disable);
    void                        disableP90Curve(bool disable);
    void                        disableMeanCurve(bool disable);

    virtual void                            fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual void                            defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

private:
    RimEnsembleCurveSet*        parentCurveSet() const;

    caf::PdmField<bool>         m_active;
    caf::PdmField<bool>         m_basedOnFilteredCases;
    caf::PdmField<bool>         m_showP10Curve;
    caf::PdmField<bool>         m_showP50Curve;
    caf::PdmField<bool>         m_showP90Curve;
    caf::PdmField<bool>         m_showMeanCurve;
    caf::PdmField<bool>         m_showCurveLabels;

    caf::PdmField<cvf::Color3f> m_color;

    RimSummaryCaseCollection*   m_ensemble;
};

