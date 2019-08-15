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

#include "RifSummaryReaderInterface.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

class QDateTime;
class QString;

class RifKeywordVectorParser;
class RifEclipseSummaryAddress;

//==================================================================================================
//
//
//==================================================================================================
class RifKeywordVectorUserData : public RifSummaryReaderInterface
{
public:
    RifKeywordVectorUserData();
    ~RifKeywordVectorUserData() override;

    bool                                parse(const QString& data, const QString& customWellName);

    const std::vector<time_t>&  timeSteps(const RifEclipseSummaryAddress& resultAddress) const override;

    bool                        values(const RifEclipseSummaryAddress& resultAddress,
                                               std::vector<double>* values) const override;

    std::string                     unitName(const RifEclipseSummaryAddress& resultAddress) const override;
    RiaEclipseUnitTools::UnitSystem unitSystem() const override;

private:
    static bool                         isTimeHeader(const std::map<QString, QString>& header);
    static bool                         isVectorHeader(const std::map<QString, QString>& header);
    static QString                      valueForKey(const std::map<QString, QString>& header, const QString& key);

private:
    std::unique_ptr<RifKeywordVectorParser>     m_parser;

    std::vector< std::vector<time_t> >          m_timeSteps;
    
    std::map<RifEclipseSummaryAddress, size_t>  m_mapFromAddressToVectorIndex;
    std::map<RifEclipseSummaryAddress, size_t>  m_mapFromAddressToTimeIndex;
};
