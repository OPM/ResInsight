/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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
#include "cafPdmObject.h"
#include "cafSignal.h"

#include <functional>
#include <vector>

class RimSummaryCase;
class RimFileSummaryCase;
class RimEclipseResultCase;
class RimSummaryEnsemble;
class RifSummaryCaseFileResultInfo;

//==================================================================================================
///
//==================================================================================================
class RimSummaryCaseMainCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    caf::Signal<> dataSourceHasChanged;

public:
    RimSummaryCaseMainCollection();
    ~RimSummaryCaseMainCollection() override;

    RimSummaryCase* summaryCase( size_t idx );
    size_t          summaryCaseCount() const;

    std::vector<RimSummaryCase*>     allSummaryCases() const;
    std::vector<RimSummaryCase*>     topLevelSummaryCases() const;
    std::vector<RimSummaryEnsemble*> summaryEnsembles() const;

    std::vector<RimSummaryCase*> createSummaryCasesFromFileInfos( const std::vector<RifSummaryCaseFileResultInfo>& summaryHeaderFileInfos,
                                                                  bool                                             showProgress = false );

    RimSummaryCase* findTopLevelSummaryCaseFromFileName( const QString& fileName ) const;

    void addCases( const std::vector<RimSummaryCase*> cases );
    void addCase( RimSummaryCase* summaryCase );
    void removeCase( RimSummaryCase* summaryCase, bool notifyChange = true );
    void removeCases( std::vector<RimSummaryCase*>& cases );
    void moveCase( RimSummaryCase* summaryCase, int destinationIndex );

    RimSummaryEnsemble* addEnsemble( const std::vector<RimSummaryCase*>&  summaryCases,
                                     const QString&                       coolectionName,
                                     bool                                 isEnsemble,
                                     std::function<RimSummaryEnsemble*()> allocator = defaultAllocator );
    void                removeEnsemble( RimSummaryEnsemble* ensemble );
    void                addEnsemble( RimSummaryEnsemble* ensemble );
    void                moveEnsemble( RimSummaryEnsemble* ensemble, int destinationIndex );

    void loadAllSummaryCaseData();

    QString uniqueShortNameForCase( RimSummaryCase* summaryCase );

    void updateAutoShortName();
    void onProjectBeingSaved();

    void updateEnsembleNames();

private:
    void initAfterRead() override;

    static void                loadSummaryCaseData( const std::vector<RimSummaryCase*>& summaryCases );
    static void                loadFileSummaryCaseData( const std::vector<RimFileSummaryCase*>& fileSummaryCases );
    static RimSummaryEnsemble* defaultAllocator();

    void onCaseNameChanged( const SignalEmitter* emitter );

private:
    caf::PdmChildArrayField<RimSummaryCase*>     m_cases;
    caf::PdmChildArrayField<RimSummaryEnsemble*> m_ensembles;
};
