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

#include "RiaEclipseUnitTools.h"
#include "RifEclipseSummaryAddress.h"
#include "RifSummaryReaderInterface.h"

#include <QString>
#include <QStringList>

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

//==================================================================================================
//
//
//==================================================================================================
class RifRestartFileInfo
{
public:
    RifRestartFileInfo()
        : startDate( 0 )
        , endDate( 0 )
    {
    }
    RifRestartFileInfo( const QString& _fileName, time_t _startDate, time_t _endDate )
        : fileName( _fileName )
        , startDate( _startDate )
        , endDate( _endDate )
    {
    }
    bool valid() { return !fileName.isEmpty(); }

    QString fileName;
    time_t  startDate;
    time_t  endDate;
};

//==================================================================================================
//
//
//==================================================================================================
class RifReaderEclipseSummary : public RifSummaryReaderInterface
{
public:
    RifReaderEclipseSummary();
    ~RifReaderEclipseSummary() override;

    bool open( const QString& headerFileName, bool includeRestartFiles );

    std::vector<RifRestartFileInfo> getRestartFiles( const QString& headerFileName, bool* hasWarnings );
    RifRestartFileInfo              getFileInfo( const QString& headerFileName );

    const std::vector<time_t>& timeSteps( const RifEclipseSummaryAddress& resultAddress ) const override;

    bool        values( const RifEclipseSummaryAddress& resultAddress, std::vector<double>* values ) const override;
    std::string unitName( const RifEclipseSummaryAddress& resultAddress ) const override;
    RiaEclipseUnitTools::UnitSystem unitSystem() const override;
    QStringList                     warnings() const { return m_warnings; }

    void markForCachePurge( const RifEclipseSummaryAddress& address ) override;

    static std::string       differenceIdentifier() { return "_DIFF"; }
    static const std::string historyIdentifier() { return "H"; }

private:
    int                timeStepCount() const;
    int                indexFromAddress( const RifEclipseSummaryAddress& resultAddress ) const;
    void               buildMetaData();
    RifRestartFileInfo getRestartFile( const QString& headerFileName );

private:
    // Taken from ecl_sum.h
    typedef struct ecl_sum_struct    ecl_sum_type;
    typedef struct ecl_smspec_struct ecl_smspec_type;

    ecl_sum_type*          m_ecl_sum;
    const ecl_smspec_type* m_ecl_SmSpec;
    std::vector<time_t>    m_timeSteps;

    RiaEclipseUnitTools::UnitSystem m_unitSystem;

    std::map<RifEclipseSummaryAddress, int> m_resultAddressToErtNodeIdx;

    QStringList m_warnings;

    std::set<RifEclipseSummaryAddress> m_differenceAddresses;

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
        void                       markAddressForPurge( const RifEclipseSummaryAddress& address );

    private:
        void purgeData();

        std::map<const RifEclipseSummaryAddress, std::vector<double>> m_cachedValues;
        std::set<RifEclipseSummaryAddress>                            m_purgeList;
    };

    std::unique_ptr<ValuesCache> m_valuesCache;
};
