/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "RifSummaryReaderInterface.h"

#include "RimSummaryCase.h"

#include "cafPdmPointer.h"

class RimSummaryEnsembleSumo;

//==================================================================================================
//
//
//
//==================================================================================================
class RimSummaryCaseSumo : public RimSummaryCase, public RifSummaryReaderInterface
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryCaseSumo();

    void setEnsemble( RimSummaryEnsembleSumo* ensemble );
    void setValues( const std::vector<time_t>& timeSteps, const RifEclipseSummaryAddress& resultAddress, const std::vector<float>& values );

    int32_t realizationNumber() const;
    void    setRealizationNumber( int32_t realizationNumber );

    QString realizationName() const;
    void    setRealizationName( const QString& realizationName );

    void                       createSummaryReaderInterface() override;
    RifSummaryReaderInterface* summaryReader() override;

    std::vector<time_t>                  timeSteps( const RifEclipseSummaryAddress& resultAddress ) const override;
    std::pair<bool, std::vector<double>> values( const RifEclipseSummaryAddress& resultAddress ) const override;
    std::string                          unitName( const RifEclipseSummaryAddress& resultAddress ) const override;
    RiaDefines::EclipseUnitSystem        unitSystem() const override;

protected:
    QString caseName() const override;
    void    createAndSetAddresses() override;
    size_t  keywordCount() const override;

private:
    caf::PdmPointer<RimSummaryEnsembleSumo>                m_ensemble;
    QString                                                m_realizationName;
    int32_t                                                m_realizationNumber;
    std::vector<time_t>                                    m_timeSteps;
    std::map<RifEclipseSummaryAddress, std::vector<float>> m_values;
};
