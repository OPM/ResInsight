/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
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

#include "RiaDefines.h"

#include "RifSummaryReaderInterface.h"

#include <QString>
#include <QStringList>

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

class RifOpmCommonEclipseSummary;
class RifOpmHdf5Summary;
class RifEclEclipseSummary;
class RiaThreadSafeLogger;
class RifEclipseSummaryAddress;

//==================================================================================================
//
//
//==================================================================================================
class RifReaderEclipseSummary : public RifSummaryReaderInterface
{
public:
    RifReaderEclipseSummary();
    ~RifReaderEclipseSummary() override;

    bool open( const QString& headerFileName, RiaThreadSafeLogger* threadSafeLogger );

    const std::vector<time_t>& timeSteps( const RifEclipseSummaryAddress& resultAddress ) const override;

    bool        values( const RifEclipseSummaryAddress& resultAddress, std::vector<double>* values ) const override;
    std::string unitName( const RifEclipseSummaryAddress& resultAddress ) const override;
    RiaDefines::EclipseUnitSystem unitSystem() const override;

    static std::string       differenceIdentifier() { return "_DIFF"; }
    static const std::string historyIdentifier() { return "H"; }

private:
    void buildMetaData();

    RifSummaryReaderInterface* currentSummaryReader() const;

private:
    std::unique_ptr<RifSummaryReaderInterface> m_summaryReader;
    std::set<RifEclipseSummaryAddress>         m_differenceAddresses;

private:
    //==================================================================================================
    //
    //==================================================================================================
    class ValuesCache
    {
        static const std::vector<double> EMPTY_VECTOR;

    public:
        ValuesCache();
        ~ValuesCache();

        void insertValues( const RifEclipseSummaryAddress& address, const std::vector<double>& values );
        const std::vector<double>& getValues( const RifEclipseSummaryAddress& address ) const;

    private:
        std::map<const RifEclipseSummaryAddress, std::vector<double>> m_cachedValues;
    };

    std::unique_ptr<ValuesCache> m_valuesCache;
};
