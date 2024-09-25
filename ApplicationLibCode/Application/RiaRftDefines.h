/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022- Equinor ASA
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

#include <QString>

namespace RiaDefines
{
bool    isSegmentResult( const QString& resultName );
QString segmentStartDepthResultName();
QString segmentEndDepthResultName();
QString segmentTvdDepthResultName();
QString segmentNumberResultName();

bool        isSegmentConnectionResult( const QString& resultName );
QString     segmentConnectionTvdDepthResultName();
std::string segmentConnectionEndDepthResultName();
std::string segmentConnectionStartDepthResultName();
std::string segmentConnectionBranchNoResultName();
QString     segmentConnectionMeasuredDepthResultName();
std::string segmentConnectionIPos();
std::string segmentConnectionJPos();
std::string segmentConnectionKPos();

QString allBranches();
QString segmentBranchNumberResultName();

enum class RftBranchType
{
    RFT_TUBING,
    RFT_DEVICE,
    RFT_ANNULUS,
    RFT_UNKNOWN
};

}; // namespace RiaDefines
