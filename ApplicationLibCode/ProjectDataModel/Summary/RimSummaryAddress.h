/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RiaDefines.h"

#include "RifEclipseSummaryAddressQMetaType.h"

#include "cafAppEnum.h"

class RifSummaryReaderInterface;
class RimSummaryCase;
class RimSummaryFilter_OBSOLETE;
class RiuQwtPlotCurve;

class RimSummaryAddress : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryAddress();
    ~RimSummaryAddress() override;

    static RimSummaryAddress* wrapFileReaderAddress( const RifEclipseSummaryAddress& addr, int caseId = -1, int ensembleId = -1 );

    void                     setAddress( const RifEclipseSummaryAddress& addr );
    RifEclipseSummaryAddress address() const;

    void setCaseId( int caseId );
    int  caseId() const;

    void setEnsembleId( int ensembleId );
    int  ensembleId() const;
    bool isEnsemble() const;

    QString quantityName() const;

    RiaDefines::PhaseType addressPhaseType() const;

    QString keywordForCategory( RifEclipseSummaryAddressDefines::SummaryCategory category ) const;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    QString iconResourceText() const;
    void    initAfterRead() override;

private:
    caf::PdmField<caf::AppEnum<RifEclipseSummaryAddressDefines::SummaryCategory>> m_category;
    caf::PdmField<caf::AppEnum<RifEclipseSummaryAddressDefines::StatisticsType>>  m_statistics;

    caf::PdmField<QString> m_vectorName;
    caf::PdmField<int>     m_regionNumber;
    caf::PdmField<int>     m_regionNumber2;
    caf::PdmField<QString> m_groupName;
    caf::PdmField<QString> m_networkName;
    caf::PdmField<QString> m_wellName;
    caf::PdmField<int>     m_wellSegmentNumber;
    caf::PdmField<QString> m_lgrName;
    caf::PdmField<int>     m_cellI;
    caf::PdmField<int>     m_cellJ;
    caf::PdmField<int>     m_cellK;
    caf::PdmField<int>     m_aquiferNumber;
    caf::PdmField<int>     m_wellCompletionNumber;
    caf::PdmField<bool>    m_isErrorResult;
    caf::PdmField<int>     m_calculationId;
    caf::PdmField<int>     m_caseId;
    caf::PdmField<int>     m_ensembleId;
};
