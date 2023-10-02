/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023-     Equinor ASA
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

#include "RimWellLogFileUtil.h"

#include "RimWellLogFile.h"
#include "RimWellPath.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<double, double>> RimWellLogFileUtil::findMdAndChannelValuesForWellPath( const RimWellPath& wellPath,
                                                                                              const QString&     channelName,
                                                                                              QString*           unitString /*=nullptr*/ )
{
    std::vector<RimWellLogFile*> wellLogFiles = wellPath.descendantsIncludingThisOfType<RimWellLogFile>();
    for ( RimWellLogFile* wellLogFile : wellLogFiles )
    {
        auto values = wellLogFile->findMdAndChannelValuesForWellPath( wellPath, channelName, unitString );
        if ( !values.empty() ) return values;
    }

    return std::vector<std::pair<double, double>>();
}
