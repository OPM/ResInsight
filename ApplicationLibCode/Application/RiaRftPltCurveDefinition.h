/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RifDataSourceForRftPlt.h"

#include <QString>

#include <utility>
#include <vector>

class RimSummaryCase;

//==================================================================================================
///
//==================================================================================================
class RiaRftPltCurveDefinition
{
public:
    explicit RiaRftPltCurveDefinition( const RifDataSourceForRftPlt& address, const QString& wellName, const QDateTime& timeStep );

    const RifDataSourceForRftPlt& address() const;
    const QString&                wellName() const;
    const QDateTime&              timeStep() const;

    auto operator<=>( const RiaRftPltCurveDefinition& rhs ) const -> std::strong_ordering;

private:
    RifDataSourceForRftPlt m_curveAddress;
    QString                m_wellName;
    QDateTime              m_timeStep;
};
