/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020- Equinor ASA
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
#include "cafPdmObjectHandle.h"
#include "cafPdmObjectMethod.h"
#include "cafPdmPtrArrayField.h"
#include "cafPdmPtrField.h"

#include <QString>

#include <memory>

class RimSummaryCase;
class RimSummaryPlotCollection;
class RimSummaryEnsemble;
class RimSummaryPlot;

//==================================================================================================
///
//==================================================================================================
class RimcSummaryPlotCollection_newSummaryPlot : public caf::PdmObjectCreationMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcSummaryPlotCollection_newSummaryPlot( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

    static std::vector<RimSummaryEnsemble*> allEnsembles( RimSummaryEnsemble*                               ensemble,
                                                          const caf::PdmPtrArrayField<RimSummaryEnsemble*>& ensembleArray );

    static RimSummaryPlot* createSummaryPlot( const std::vector<RimSummaryCase*>&     summaryCases,
                                              const std::vector<RimSummaryEnsemble*>& ensembles,
                                              const QString&                          address );

private:
    caf::PdmField<QString>                     m_addressString;
    caf::PdmPtrArrayField<RimSummaryCase*>     m_summaryCases;
    caf::PdmPtrField<RimSummaryEnsemble*>      m_ensemble;
    caf::PdmPtrArrayField<RimSummaryEnsemble*> m_ensembles;
};

//==================================================================================================
///
//==================================================================================================
class RimcSummaryPlotCollection_newSummaryMultiPlot : public caf::PdmObjectCreationMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcSummaryPlotCollection_newSummaryMultiPlot( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmField<std::vector<QString>>        m_addressStrings;
    caf::PdmPtrArrayField<RimSummaryCase*>     m_summaryCases;
    caf::PdmPtrArrayField<RimSummaryEnsemble*> m_ensembles;
};
