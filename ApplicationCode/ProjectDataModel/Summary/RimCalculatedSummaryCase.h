/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017  Statoil ASA
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

#include "RimSummaryCase.h"
#include "RifSummaryReaderInterface.h"

class RimSummaryCalculationCollection;
class RimSummaryCalculation;


//==================================================================================================
//
//==================================================================================================
class RimCalculatedSummaryCase : public RimSummaryCase
{
    CAF_PDM_HEADER_INIT;

private:
    //==================================================================================================
    //
    //==================================================================================================
    class RifCalculatedSummaryCurveReader : public RifSummaryReaderInterface
    {
    public:
        explicit RifCalculatedSummaryCurveReader(RimSummaryCalculationCollection* calculationCollection);

        virtual const std::vector<time_t>&  timeSteps(const RifEclipseSummaryAddress& resultAddress) const override;
        virtual bool                        values(const RifEclipseSummaryAddress& resultAddress, std::vector<double>* values) const override;
        virtual std::string                 unitName(const RifEclipseSummaryAddress& resultAddress) const override;

        void                                buildMetaData();

    private:
        RimSummaryCalculation*                     findCalculationByName(const RifEclipseSummaryAddress& resultAddress) const;

    private:
        caf::PdmPointer<RimSummaryCalculationCollection> m_calculationCollection;
    };

public:
    RimCalculatedSummaryCase();
    virtual ~RimCalculatedSummaryCase();

    virtual QString                         caseName() override;
    virtual void                            createSummaryReaderInterface() override;
    virtual RifSummaryReaderInterface*      summaryReader() override;
    virtual void                            updateFilePathsFromProjectPath(const QString& newProjectPath, const QString& oldProjectPath) override;

    void                                    buildMetaData();

private:
    RifCalculatedSummaryCurveReader*        m_calculatedCurveReader;
};
