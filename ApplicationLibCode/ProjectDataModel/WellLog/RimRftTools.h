/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022-     Equinor ASA
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

#include "RiaRftDefines.h"
#include "RifEclipseRftAddress.h"

#include <QList>

namespace caf
{
class PdmOptionItemInfo;
}
class RifReaderRftInterface;
class RifReaderOpmRft;

class RimRftTools
{
public:
    static QList<caf::PdmOptionItemInfo> wellLogChannelsOptions( RifReaderRftInterface* readerRft, const QString& wellName );
    static QList<caf::PdmOptionItemInfo> wellNameOptions( RifReaderRftInterface* readerRft );
    static QList<caf::PdmOptionItemInfo>
        timeStepOptions( RifReaderRftInterface* readerRft, const QString& wellName, RifEclipseRftAddress::RftWellLogChannelType channelType );

    static QList<caf::PdmOptionItemInfo> segmentTimeStepOptions( RifReaderRftInterface* readerRft, const QString& wellName );

    static QList<caf::PdmOptionItemInfo>
        segmentResultNameOptions( RifReaderRftInterface* readerRft, const QString& wellName, const QDateTime& timeStep );

    static QList<caf::PdmOptionItemInfo> segmentBranchIndexOptions( RifReaderOpmRft*          readerRft,
                                                                    const QString&            wellName,
                                                                    const QDateTime&          timeStep,
                                                                    RiaDefines::RftBranchType branchType );

    static std::vector<double> segmentStartMdValues( RifReaderOpmRft*          readerRft,
                                                     const QString&            wellName,
                                                     const QDateTime&          dateTime,
                                                     int                       segmentBranchIndex,
                                                     RiaDefines::RftBranchType segmentBranchType );

    static std::vector<double> segmentEndMdValues( RifReaderOpmRft*          readerRft,
                                                   const QString&            wellName,
                                                   const QDateTime&          dateTime,
                                                   int                       segmentBranchIndex,
                                                   RiaDefines::RftBranchType segmentBranchType );

    static std::vector<double> segmentConnectionMdValues( RifReaderOpmRft*          readerRft,
                                                          const QString&            wellName,
                                                          const QDateTime&          dateTime,
                                                          int                       segmentBranchIndex,
                                                          RiaDefines::RftBranchType segmentBranchType );
};
