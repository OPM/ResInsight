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

#include <vector>
#include <functional>

class RimGridSummaryCase;
class RimSummaryCase;
class RimEclipseResultCase;
class RimSummaryCaseCollection;
class RifSummaryCaseFileResultInfo;

//==================================================================================================
/// 
//==================================================================================================
class RimSummaryCaseMainCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimSummaryCaseMainCollection();
    virtual ~RimSummaryCaseMainCollection();

    RimSummaryCase*     summaryCase(size_t idx);
    size_t              summaryCaseCount() const;

    std::vector<RimSummaryCase*>            allSummaryCases() const;
    std::vector<RimSummaryCase*>            topLevelSummaryCases() const;
    std::vector<RimSummaryCaseCollection*>  summaryCaseCollections() const;

    std::vector<RimSummaryCase*> createSummaryCasesFromFileInfos(const std::vector<RifSummaryCaseFileResultInfo>& summaryHeaderFileInfos, bool showProgress = false);

    RimSummaryCase*     findSummaryCaseFromEclipseResultCase(const RimEclipseResultCase* eclResCase) const;
    RimSummaryCase*     findSummaryCaseFromFileName(const QString& fileName) const;
    void                convertGridSummaryCasesToFileSummaryCases(RimGridSummaryCase* gridSummaryCase);
 
    void                addCases(const std::vector<RimSummaryCase*> cases);
    void                addCase(RimSummaryCase* summaryCase);
    void                removeCase(RimSummaryCase* summaryCase);

    RimSummaryCaseCollection* addCaseCollection(std::vector<RimSummaryCase*> summaryCases,
                                          const QString& coolectionName,
                                          bool isEnsemble,
                                          std::function<RimSummaryCaseCollection* ()> allocator = defaultAllocator);
    void                removeCaseCollection(RimSummaryCaseCollection* caseCollection);

    void                loadAllSummaryCaseData();

    QString             uniqueShortNameForCase(RimSummaryCase* summaryCase);

    void                updateFilePathsFromProjectPath(const QString& newProjectPath, const QString& oldProjectPath);

private:
    static void         loadSummaryCaseData(std::vector<RimSummaryCase*> summaryCases);
    static RimSummaryCaseCollection* defaultAllocator();

private:
    caf::PdmChildArrayField<RimSummaryCase*> m_cases;
    caf::PdmChildArrayField<RimSummaryCaseCollection*> m_caseCollections;
};
