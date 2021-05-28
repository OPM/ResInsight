/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021- Equinor ASA
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

#include <vector>

class QString;

class RimWellLogFile;

//==================================================================================================
///
//==================================================================================================
class RimEnsembleWellLogStatistics
{
public:
    enum class StatisticsType
    {
        P10,
        P50,
        P90,
        MEAN
    };

    RimEnsembleWellLogStatistics();

    const std::vector<double>& measuredDepths() const;
    const std::vector<double>& tvDepths() const;
    const std::vector<double>& p10() const;
    const std::vector<double>& p50() const;
    const std::vector<double>& p90() const;
    const std::vector<double>& mean() const;

    bool hasP10Data() const;
    bool hasP50Data() const;
    bool hasP90Data() const;
    bool hasMeanData() const;

    void calculate( const std::vector<RimWellLogFile*>& sumCases, const QString& wellLogChannelName );

private:
    void clearData();

    std::vector<double> m_measuredDepths;
    std::vector<double> m_tvDepths;
    std::vector<double> m_p10Data;
    std::vector<double> m_p50Data;
    std::vector<double> m_p90Data;
    std::vector<double> m_meanData;
};
